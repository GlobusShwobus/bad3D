#include "Application.h"
#include "Utils.h"

Application::Application(const WINDOW_CREATE_DESC& window_desc)
{
	D3D12_COMMAND_QUEUE_DESC command_queue_desc = {};
	command_queue_desc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;            // command list type and command queue types must match. generally either: direct, compute or copy but there are others 
	command_queue_desc.Priority = D3D12_COMMAND_QUEUE_PRIORITY_NORMAL;   // for rendering normal, for non sequential use high priority ( not sure for what currently )
	command_queue_desc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;            // enable / disable GPU timeouts. keep default enabled
	command_queue_desc.NodeMask = 0;                                     // for multi adapter systems

	// info queue description
	D3D12_INFO_QUEUE_FILTER info_queue_deny_filter = {};
	{
		// Suppress whole categories of messages
		// D3D12_MESSAGE_CATEGORY Categories[] = {};
		D3D12_MESSAGE_SEVERITY Severities[] =
		{
			D3D12_MESSAGE_SEVERITY_INFO
		};

		// Suppress individual messages by their ID
		D3D12_MESSAGE_ID DenyIds[] = {
			D3D12_MESSAGE_ID_CLEARRENDERTARGETVIEW_MISMATCHINGCLEARVALUE,   // I'm really not sure how to avoid this message.
			D3D12_MESSAGE_ID_MAP_INVALID_NULLRANGE,                         // This warning occurs when using capture frame while graphics debugging.
			D3D12_MESSAGE_ID_UNMAP_INVALID_NULLRANGE,                       // This warning occurs when using capture frame while graphics debugging.
		};

		//info_queue_deny_filter.DenyList.NumCategories = _countof(Categories);
		//info_queue_deny_filter.DenyList.pCategoryList = Categories;
		info_queue_deny_filter.DenyList.NumSeverities = _countof(Severities);
		info_queue_deny_filter.DenyList.pSeverityList = Severities;
		info_queue_deny_filter.DenyList.NumIDs = _countof(DenyIds);
		info_queue_deny_filter.DenyList.pIDList = DenyIds;
	}

	// create DXGI factory
	UINT create_factory_flags = 0;  // only 2 values are valid: 0 or debug.
#if defined(_DEBUG)
	create_factory_flags = DXGI_CREATE_FACTORY_DEBUG;
#endif
	execute_and_test_hresult(
		CreateDXGIFactory2(create_factory_flags, IID_PPV_ARGS(&mFactory))
	);

	// find a good adapter
	DXGIAdapter4 adapter4;
	execute_and_test_hresult(
		find_adapter(mFactory.Get(), using_WARP_adapter, adapter4)
	);

	// create device
	execute_and_test_hresult(
		D3D12CreateDevice(adapter4.Get(), D3D_FEATURE_LEVEL_11_0, IID_PPV_ARGS(&mDevice))
	);

#if defined(_DEBUG)
	// set device debug info
	D3D12InfoQueue infoQueue;
	if (SUCCEEDED(mDevice.As(&infoQueue)))
	{
		// set break point for types
		infoQueue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_CORRUPTION, TRUE);
		infoQueue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_ERROR, TRUE);
		infoQueue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_WARNING, TRUE);

		// ignore messages
		execute_and_test_hresult(
			infoQueue->PushStorageFilter(&info_queue_deny_filter)
		);
	}
#endif

	// create command queue
	mCommandQueue = std::make_unique<DX12CommandQueue>(command_queue_desc, mDevice.Get());

	// set all trackers to 0 initally
	mSignalTracker[0] = 0ull;
	mSignalTracker[1] = 0ull;
	mSignalTracker[2] = 0ull;

	// create window, should throw internally if fails
	mDXWindow = std::make_unique<DX12RenderWindow>(window_desc, mFactory.Get(), mDevice.Get(), mCommandQueue.get(), number_of_back_buffers);
	mDXWindow->bind_listener(this);

	mIsRuntimeSet = true;
}

Application::~Application()
{
	mIsRuntimeSet = false;
	mIsGameBound = false;

	mCommandQueue->flush();

	if (mGame)
	{
		mGame->unload_content();
		mGame.reset();
	}

	mCommandQueue.reset();
	mDXWindow.reset();
	mCommandList.observe_this(nullptr);
	mDevice.Reset();
	mFactory.Reset();

	mTimer.reset();
}

LRESULT Application::on_message(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	// is_init is an important protection layer making sure no application events are processed before construction is done ( async bs )

	switch (uMsg)
	{
	case WM_DESTROY:
		PostQuitMessage(0);
		mIsRuntimeSet = false;
		return 0;

	case WM_SIZE:

		if (wParam != SIZE_MINIMIZED)
		{
			mDXWindow->on_resize();
		}
		break;

	case WM_SYSKEYDOWN:
	case WM_KEYDOWN:

		if (wParam == VK_F11)
		{
			mDXWindow->on_fullscreen_transition();
		}

		// note: intentional fall through
	case WM_SYSKEYUP:
	case WM_KEYUP:

		mGame->on_key_event(uMsg, wParam, lParam);
		break;


	case WM_LBUTTONDOWN:
	case WM_LBUTTONUP:
	case WM_RBUTTONDOWN:
	case WM_RBUTTONUP:
	case WM_MBUTTONDOWN:
	case WM_MBUTTONUP:
	case WM_XBUTTONDOWN:
	case WM_XBUTTONUP:
	case WM_MOUSEWHEEL:
	case WM_MOUSEMOVE:

		mGame->on_mouse_event(uMsg, wParam, lParam);
		break;

	default:
		break; // default breaks switch and goes to DefWindowProc
	}

	return DefWindowProcW(hwnd, uMsg, wParam, lParam);
}

void Application::bind_game(std::unique_ptr<IGame> game)
{
	if (mGame) {
		mCommandQueue->flush(); // make sure gpu is finished with resources before freeing them
		mGame->unload_content();
	}
	if (!game)
	{
		mGame.reset();
		mTimer.reset();
		mIsGameBound = false;
		return;
	}
	// assign game
	mGame = std::move(game);
	mGame->load_content();
	// set is init which up until this point protecd against sytem commands
	mTimer.reset();
	mIsGameBound = true;
}

void Application::run()
{
	if (!mTimer)
		mTimer = std::make_unique<Stopwatch>();

	while (mIsRuntimeSet && mIsGameBound)
	{
		const float dt = mTimer->dt_float();
		// call wndproc
		MSG msg = {};
		while (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE))
		{
			DispatchMessage(&msg);
		}

		mGame->on_update(dt);

		reset_rendering();

		on_user_render();

		finish_rendering();
	}
}

void Application::reset_rendering()
{
	// reset command list
	mCommandList = mCommandQueue->get_command_list();

	// set the current back buffer to go from PRESENT to RTV
	auto buffer = mDXWindow->get_buffer();
	D3D12_RESOURCE_BARRIER barrier = {};
	barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
	barrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
	barrier.Transition.pResource = buffer.get();
	barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_PRESENT;         // this knowledge is implied and for back buffers it's fine because they're only ever in one or the other state. for other resources that get more complex logic, state tracking becomes important
	barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_RENDER_TARGET;
	barrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
	mCommandList->ResourceBarrier(1, &barrier);
}

void Application::finish_rendering()
{
	// set the current back buffer to go from RTV to PRESENT
	auto buffer = mDXWindow->get_buffer();
	D3D12_RESOURCE_BARRIER barrier = {};
	barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
	barrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
	barrier.Transition.pResource = buffer.get();
	barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_RENDER_TARGET;         // this knowledge is implied and for back buffers it's fine because they're only ever in one or the other state. for other resources that get more complex logic, state tracking becomes important
	barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_PRESENT;
	barrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
	mCommandList->ResourceBarrier(1, &barrier);

	// the final command to the list has been recorded. close the list.
	mCommandList->Close();


	// ask for the current back buffer index, 
	// then execute and get signal value,
	// then cache signal value,
	// then present which updates current index and reseats back buffer which means i must potentially stall the CPU if the whatever new back buffer is not actually free yet
	// then check signal value before resuing some unknown buffer
	const UINT64 current_index = mDXWindow->get_buffer_index();
	const UINT64 signal_val = mCommandQueue->execute();
	mSignalTracker[current_index] = signal_val;

	mDXWindow->present();

	const UINT64 some_new_buffer_index = mDXWindow->get_buffer_index();

	mCommandQueue->wait(mSignalTracker[some_new_buffer_index]);
}

void Application::reset_signal_tracker()
{
	const UINT current = mDXWindow->get_buffer_index();
	for (UINT i = 0; i < number_of_back_buffers; i++)
		mSignalTracker[i] = mSignalTracker[current];
}

void Application::on_user_render()
{
	RenderFrameContext context;
	context.command_list = mCommandList.get();
	context.resource_desc = mDXWindow->get_buffer_desc();

	mGame->on_render(context);
}