#include "Application.h"
#include "Utils.h"
#include <stdexcept>


Application::~Application()
{
	assert(!dx12_initalised && "Application::shutdown() was not called before exit");
}

void Application::initialise(const std::wstring& title, UINT x, UINT y, UINT client_width, UINT client_height, DWORD window_style, HINSTANCE hInstance)
{
	initialise_dx12();
	initialise_render_window(title, x, y, client_width, client_height, window_style, hInstance);
	// if no exceptions up until this point, it should be all good
	dx12_initalised = true;
}

void Application::shutdown()
{
	assert(dx12_initalised && "Application must be initalized before shutdown()");

	// first flush the GPU
	flush();

	// game no longer needs graphics resources
	mGame = nullptr;

	// destroy the swap cahin before HWND and before GPU command queues (in case the swap chain would reference command queues in the future)
	mRenderWindow.reset();

	// destroy the command queues
	mDirectCommandQueue.reset();
	mComputeCommandQueue.reset();
	mCopyCommandQueue.reset();

	// destroy HWND
	if (mHwnd)
	{
		::DestroyWindow(mHwnd);
		mHwnd = nullptr;
	}

	// destroy device and DXGI
	mDevice.Reset();
	mFactory.Reset();

	dx12_initalised = false;
}

void Application::flush()
{
	mDirectCommandQueue->flush();
	mComputeCommandQueue->flush();
	mCopyCommandQueue->flush();
}

void Application::run()
{
	assert(dx12_initalised && "Application must be initalised before run()");

	bool running = true;

	while (running)
	{
		MSG msg = {};
		while (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE))
		{
			if (msg.message == WM_QUIT)
			{
				running = false;
				break;
			}

			DispatchMessage(&msg);
		}

		if (!running) // WM_DESTROY may have fired inside DispatchMessage in which case ignore further code
			break;

		if (mGame) {
			mGame->on_update();
			mGame->on_render();
		}
	}
}

Microsoft::WRL::ComPtr<IDXGIAdapter4> Application::find_adapter(ViewPtr<IDXGIFactory4> factory, bool use_warp)
{
	assert(factory && "factory nullptr");

	HRESULT hr = E_FAIL;
	Microsoft::WRL::ComPtr<IDXGIAdapter4> adapter4;
	if (use_warp) // since WARP is a specific adapter, just get it directly. EnumWarpAdapter takes type void as param, so query interface works as expected.
	{
		hr = factory->EnumWarpAdapter(IID_PPV_ARGS(&adapter4));
	}
	else         // if not using WARP, need to look for an adapter
	{
		// first, if looking for adapter manually, it is not possible to enumerate with Adapter4 since EnumAdapters and EnumAdapters1 take specific types.
		// secondly, need to find adapter with a good amount of memory...
		LUID best_luid = {};
		Microsoft::WRL::ComPtr<IDXGIAdapter1> adapter1;
		SIZE_T largest_memory_pool = 0;

		for (UINT adapterIndex = 0; ; ++adapterIndex)
		{
			// if reached end of the line
			if (factory->EnumAdapters1(adapterIndex, &adapter1) == DXGI_ERROR_NOT_FOUND)
				break;

			DXGI_ADAPTER_DESC1 desc1;
			adapter1->GetDesc1(&desc1);

			// ignore software adapters
			if ((desc1.Flags & DXGI_ADAPTER_FLAG_SOFTWARE) == 0)
			{
				// call create device to check if it succeeds but don't instantiate the type, by passing nullptr to output
				if (SUCCEEDED(D3D12CreateDevice(adapter1.Get(), D3D_FEATURE_LEVEL_11_0, __uuidof(ID3D12Device), nullptr)))
				{
					// if create device runs successfully then store the dedicated mem size and LUID and later actually enumerate the adapter by LUID
					if (desc1.DedicatedVideoMemory > largest_memory_pool)
					{
						largest_memory_pool = desc1.DedicatedVideoMemory;
						best_luid = desc1.AdapterLuid;
					}
				}
			}

			adapter1.Reset();
		}

		// enumerate adapter by the best LUID
		hr = factory->EnumAdapterByLuid(best_luid, IID_PPV_ARGS(&adapter4));
	}

	return adapter4;
}

void Application::initialise_dx12()
{
	if (dx12_initalised)
		return;

	// create DXGI factory
	UINT create_factory_flags = 0;  // only 2 values are valid: 0 or debug.
#if defined(_DEBUG)
	create_factory_flags = DXGI_CREATE_FACTORY_DEBUG;
#endif
	execute_and_test_hresult(
		CreateDXGIFactory2(create_factory_flags, IID_PPV_ARGS(&mFactory))
	);

	// find a good adapter
	Microsoft::WRL::ComPtr<IDXGIAdapter4> adapter4 = find_adapter(mFactory.Get(), using_WARP_adapter);
	if (!adapter4)
		throw std::runtime_error("failed to find adapter");

	// create device
	execute_and_test_hresult(
		D3D12CreateDevice(adapter4.Get(), D3D_FEATURE_LEVEL_11_0, IID_PPV_ARGS(&mDevice))
	);

	// if debug mode then set some triggers for easier debugging (>easier kek)
#if defined(_DEBUG)

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

	// set device debug info
	Microsoft::WRL::ComPtr<ID3D12InfoQueue> infoQueue;
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

	mDirectCommandQueue = std::make_unique<CommandQueue>(mDevice.Get(), D3D12_COMMAND_LIST_TYPE_DIRECT);
	mComputeCommandQueue = std::make_unique<CommandQueue>(mDevice.Get(), D3D12_COMMAND_LIST_TYPE_COMPUTE);
	mCopyCommandQueue = std::make_unique<CommandQueue>(mDevice.Get(), D3D12_COMMAND_LIST_TYPE_COPY);
}

void Application::initialise_render_window(const std::wstring& title, UINT x, UINT y, UINT client_width, UINT client_height, DWORD window_style, HINSTANCE hInstance)
{
	WNDCLASSEX register_desc = {};
	register_desc.cbSize = sizeof(WNDCLASSEX);
	register_desc.lpszClassName = L"DX12RenderWindow";
	register_desc.lpfnWndProc = Application::wnd_proc;
	register_desc.hInstance = hInstance;
	register_desc.style = CS_HREDRAW | CS_VREDRAW;
	register_desc.hIcon = nullptr;
	register_desc.hIconSm = nullptr;
	register_desc.hCursor = nullptr;
	register_desc.hbrBackground = nullptr;
	register_desc.lpszMenuName = nullptr;
	register_desc.cbClsExtra = 0;
	register_desc.cbWndExtra = 0;

	if (RegisterClassExW(&register_desc) == 0)
	{
		if (::GetLastError() != ERROR_CLASS_ALREADY_EXISTS)
			throw std::runtime_error("class register failure");
	}

	// adjust client size to window size and create the window
	RECT window_rect{ static_cast<LONG>(x), static_cast<LONG>(y),
					   static_cast<LONG>(x + client_width), static_cast<LONG>(y + client_height) };
	::AdjustWindowRect(&window_rect, window_style, FALSE);

	const int win_x = static_cast<int>(std::max<LONG>(window_rect.left, 0));
	const int win_y = static_cast<int>(std::max<LONG>(window_rect.top, 0));
	const int win_w = static_cast<int>(rect_width(window_rect));
	const int win_h = static_cast<int>(rect_height(window_rect));

	HWND hwnd = CreateWindowExW(
		NULL,
		register_desc.lpszClassName,
		title.c_str(),
		window_style,
		win_x,
		win_y,
		win_w,
		win_h,
		NULL,
		NULL,
		register_desc.hInstance,
		this
	);

	if (!hwnd)
		throw std::runtime_error("class creation failed");

	// disable alt + enter because fullscreen / windowed transitions are manual
	execute_and_test_hresult(
		mFactory->MakeWindowAssociation(hwnd, DXGI_MWA_NO_ALT_ENTER)
	);

	// make swap chain
	mRenderWindow = std::make_unique<RenderWindow>(hwnd, mFactory.Get(), mDevice.Get(), mDirectCommandQueue->get_queue(), window_style);

	// show
	::ShowWindow(hwnd, SW_SHOW);
}

LRESULT Application::on_message(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch (uMsg)
	{
	case WM_DESTROY:
		PostQuitMessage(0);
		break;

	case WM_SIZE:

		if (wParam != SIZE_MINIMIZED && mGame)
		{
			mGame->on_resize();
		}
		break;

	case WM_SYSKEYDOWN:
	case WM_KEYDOWN:
	case WM_SYSKEYUP:
	case WM_KEYUP:

		if(mGame)
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

		if(mGame)
			mGame->on_mouse_event(uMsg, wParam, lParam);
		break;

	default:
		break; // default breaks switch and goes to DefWindowProc
	}

	return DefWindowProcW(hwnd, uMsg, wParam, lParam);
}