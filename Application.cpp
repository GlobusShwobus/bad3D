#include "Application.h"
#include <dxgi1_6.h>
#include "EasyDirectXUtils.h"
#include <assert.h>
#include "Stopwatch.h"
#include "KeyTypes.h"
Application::~Application()
{
	assert(!mInitialised && "Application::shutdown() was not called before exit");
}

void Application::initialise(AppWinDesc window_desc)
{
	if (mInitialised)
		return;

	// create DXGI factory
	Microsoft::WRL::ComPtr<IDXGIFactory4> factory4;
	UINT create_factory_flags = 0;

#if defined(_DEBUG)
	create_factory_flags = DXGI_CREATE_FACTORY_DEBUG;
#endif

	execute_and_test_hresult(
		CreateDXGIFactory2(create_factory_flags, IID_PPV_ARGS(&factory4))
	);

	assert(factory4 && "factory nullptr");

	// create adapter
	static const bool using_WARP = false;
	Microsoft::WRL::ComPtr<IDXGIAdapter4> adapter4 = find_adapter(factory4.Get(), using_WARP);

	assert(adapter4 && "adapter nullptr");

	// init stuff
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

	ViewPtr<ID3D12Device4> device_ = ViewPtr{ mDevice.Get() };
	mDirectCommandQueue = std::make_unique<CommandQueue>(device_, D3D12_COMMAND_LIST_TYPE_DIRECT);
	mComputeCommandQueue = std::make_unique<CommandQueue>(device_, D3D12_COMMAND_LIST_TYPE_COMPUTE);
	mCopyCommandQueue = std::make_unique<CommandQueue>(device_, D3D12_COMMAND_LIST_TYPE_COPY);

	WNDCLASSEX register_desc = {};
	register_desc.cbSize = sizeof(WNDCLASSEX);
	register_desc.lpszClassName = L"DX12RenderWindow";
	register_desc.lpfnWndProc = Application::wnd_proc;
	register_desc.hInstance = window_desc.hInstance;
	register_desc.style = CS_HREDRAW | CS_VREDRAW;
	register_desc.hIcon = window_desc.hIcon;
	register_desc.hIconSm = window_desc.hIconSm;
	register_desc.hCursor = window_desc.hCursor;
	register_desc.hbrBackground = nullptr;
	register_desc.lpszMenuName = nullptr;
	register_desc.cbClsExtra = 0;
	register_desc.cbWndExtra = 0;

	static ATOM atom = ::RegisterClassExW(&register_desc);
	assert(atom > 0);

	// adjust client size to window size and create the window
	RECT window_rect{ static_cast<LONG>(window_desc.x), static_cast<LONG>(window_desc.y),
					   static_cast<LONG>(window_desc.x + window_desc.cw), static_cast<LONG>(window_desc.y + window_desc.ch) };
	::AdjustWindowRect(&window_rect, window_desc.window_style, FALSE);

	const int win_x = static_cast<int>(std::max<LONG>(window_rect.left, 0));
	const int win_y = static_cast<int>(std::max<LONG>(window_rect.top, 0));
	const int win_w = static_cast<int>(rect_width(window_rect));
	const int win_h = static_cast<int>(rect_height(window_rect));

	mHwnd = CreateWindowExW(
		NULL,
		register_desc.lpszClassName,
		window_desc.window_name.c_str(),
		window_desc.window_style,
		win_x,
		win_y,
		win_w,
		win_h,
		NULL,
		NULL,
		register_desc.hInstance,
		this
	);

	assert(mHwnd && "window nullptr");

	// make swap chain
	mRenderWindow = std::make_unique<RenderWindow>(
		ViewPtr<ID3D12Device4>{mDevice.Get()},
		ViewPtr<HWND__>{mHwnd},
		mDirectCommandQueue->get_queue(),
		factory4.Get(),
		window_desc.window_style
	);

	assert(mRenderWindow && "swap chain nullptr");

	// disable alt + enter because fullscreen / windowed transitions are manual
	execute_and_test_hresult(
		factory4->MakeWindowAssociation(mHwnd, DXGI_MWA_NO_ALT_ENTER)
	);

	// show
	::ShowWindow(mHwnd, SW_SHOW);
	mInitialised = true;
}

void Application::shutdown()
{
	if (mRunning)
		throw std::runtime_error("The user must call exit_loop() before shutdown");

	if (!mInitialised)
		return;

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

	mInitialised = false;
}

void Application::flush()
{
	assert(mInitialised && "Not initalised");

	mDirectCommandQueue->flush_execution();
	mComputeCommandQueue->flush_execution();
	mCopyCommandQueue->flush_execution();
}

void Application::run()
{
	// if initalized and on the first call to run
	if (mInitialised && !mRunning)
		mRunning = true;

	Stopwatch clock;
	while (mRunning)
	{
		update_other_events(clock.dt_float());

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

LRESULT Application::on_message(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch (uMsg)
	{
	case WM_DESTROY:
		PostQuitMessage(0);
		mState.System().SetQuitEvent();
		break;

	case WM_SIZE:

		if (wParam != SIZE_MINIMIZED)
		{
			mState.System().SetResizeEvent(LOWORD(lParam), HIWORD(lParam));
		}
		break;

	case WM_SYSKEYDOWN:
	case WM_KEYDOWN:
		mState.Keyboard().SetDown(wParam);
		break;

	case WM_SYSKEYUP:
	case WM_KEYUP:
		mState.Keyboard().SetUp(wParam);
		break;

	case WM_LBUTTONDOWN:
		mState.Mouse().SetButtonDown(MouseButtonType::Left);
		break;
	case WM_LBUTTONUP:
		mState.Mouse().SetButtonUp(MouseButtonType::Left);
		break;
	case WM_RBUTTONDOWN:
		mState.Mouse().SetButtonDown(MouseButtonType::Right);
		break;
	case WM_RBUTTONUP:
		mState.Mouse().SetButtonUp(MouseButtonType::Right);
		break;
	case WM_MBUTTONDOWN:
		mState.Mouse().SetButtonDown(MouseButtonType::Middle);
		break;
	case WM_MBUTTONUP:
		mState.Mouse().SetButtonUp(MouseButtonType::Middle);
		break;
	//case WM_XBUTTONDOWN:
	//case WM_XBUTTONUP:
	case WM_MOUSEWHEEL:
		mState.Mouse().SetWheelDelta(GET_WHEEL_DELTA_WPARAM(wParam), WHEEL_DELTA);
		break;
	case WM_MOUSEMOVE:
		{
			int nx = ((int)(short)LOWORD(lParam));
			int ny = ((int)(short)HIWORD(lParam));
			int cx = mState.Mouse().PosX();
			int cy = mState.Mouse().PosY();
			
			if (nx != cx || ny != cy) // because windows can generate WM_MOUSEMOVE even when mouse seems stationary
				mState.Mouse().ResetHoverDuration();
			mState.Mouse().SetPosition(nx,ny);
		}
		break;

	default:
		break; // default breaks switch and goes to DefWindowProc
	}

	return DefWindowProcW(hwnd, uMsg, wParam, lParam);
}

void Application::update_other_events(double delta)
{
	mState.System().UpdateAge(delta);
	mState.System().ResetResizeEvent();
	mState.Mouse().UpdateHoverDuration(delta);
	mState.Mouse().ResetWheelDelta();
}

ID3D12Device4* Application::get_device() const noexcept 
{
	return mDevice.Get();
}
HWND Application::get_hwnd() const noexcept 
{ 
	return mHwnd; 
}
RenderWindow* Application::get_render_window() const noexcept 
{
	return mRenderWindow.get();
}
CommandQueue* Application::get_command_queue(D3D12_COMMAND_LIST_TYPE type) const noexcept
{
	CommandQueue* p = nullptr;

	if (type == D3D12_COMMAND_LIST_TYPE_DIRECT)
		p = mDirectCommandQueue.get();
	else if (type == D3D12_COMMAND_LIST_TYPE_COMPUTE)
		p = mComputeCommandQueue.get();
	else if (type == D3D12_COMMAND_LIST_TYPE_COPY)
		p = mCopyCommandQueue.get();

	return p;
}