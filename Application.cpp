#include "Application.h"
#include "Utils.h"
#include <stdexcept>

void Application::initialise_dx12()
{
	if (mIsDX12Initalised)
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
	DXGIAdapter4 adapter4;
	execute_and_test_hresult(
		find_adapter(mFactory.Get(), using_WARP_adapter, adapter4)
	);

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

	// create command queue for graphics
	D3D12_COMMAND_QUEUE_DESC command_queue_desc = {};
	command_queue_desc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;            // command list type and command queue types must match. generally either: direct, compute or copy but there are others 
	command_queue_desc.Priority = D3D12_COMMAND_QUEUE_PRIORITY_NORMAL;   // for rendering normal, for non sequential use high priority ( not sure for what currently )
	command_queue_desc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;            // enable / disable GPU timeouts. keep default enabled
	command_queue_desc.NodeMask = 0;                                     // for multi adapter systems

	mCommandQueue = std::make_unique<DX12CommandQueue>(command_queue_desc, mDevice.Get());

	mIsDX12Initalised = true;
}

void Application::initialise_render_window(const std::wstring& title, UINT x, UINT y, UINT client_width, UINT client_height, DWORD window_style)
{
	mDXWindow = std::make_unique<DX12RenderWindow>(
		title,
		x,
		y,
		client_width,
		client_height,
		window_style,
		mFactory.Get(),
		mDevice.Get(),
		mCommandQueue->get_observer(),
		this
	);
}

void Application::initialise_IGame(std::unique_ptr<IGame> game)
{
	if (mGame)
		throw std::runtime_error{"uninitalised IGame"};

	if (!game)
		throw std::runtime_error{"can not initialise from nullptr"};

	// assign game
	mGame = std::move(game);
	mGame->initialise(mDevice.Get(), mCommandQueue.get(), mDXWindow.get());
}

void Application::free_IGame()
{
	if (mGame)
	{
		mCommandQueue->flush();
		mGame->destroy();
	}
}

Application::~Application()
{
	mIsDX12Initalised = false;

	mCommandQueue->flush();
	
	mGame.reset();
	mCommandQueue.reset();
	mDXWindow.reset();
	mDevice.Reset();
	mFactory.Reset();
}

LRESULT Application::on_message(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch (uMsg)
	{
	case WM_DESTROY:
		PostQuitMessage(0);
		mIsDX12Initalised = false;
		return 0;

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


void Application::run()
{
	while (mIsDX12Initalised)
	{
		// call wndproc
		MSG msg = {};
		while (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE))
		{
			DispatchMessage(&msg);
		}

		if (mGame) {
			mGame->on_update();
			mGame->on_render();
		}
	}
}