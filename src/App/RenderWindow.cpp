#include "App/RenderWindow.h"

#include <stdexcept>

#include <dxgi1_6.h>

#include "D3D12/EasyDirectXUtils.h"

RenderWindow::RenderWindow(
	AppState& events,
	GraphicsDevice& device,
	RENDER_WINDOW_DESC desc
)
	:mDevice(device.get_device()),
	mQueue(device.get_direct_queue()),
	mState(events), 
	mBufferViews(device.get_device(), easy::descriptor_heap_RTV(SCONST_BACK_BUFFER_COUNT)),
	mIsFullscreen(false),
	mInitialised(false),
	mIsTearingSupported(false),
	mBufferIndex(0),
	mWidth(0),
	mHeight(0),
	mSavedWindowRect{0,0,0,0},
	mClearColor(0.0f,0.0f,0.0f,1.0f)
{
	// create HWND and set the mSavedWindowRect for fullscreen on/off toggle
	if (desc.window_name.empty() || desc.hInstance == nullptr || desc.width == 0u || desc.height == 0u)
		throw std::invalid_argument("invalid window args");

	if (!mDevice || !mQueue || !mQueue->get_queue())
		throw std::invalid_argument("invalid application args");

	if (!create_hwnd(desc))
		throw std::runtime_error("failed to init HWND ( ::GetLastError() might help)");

	::GetWindowRect(mHwnd.get(), &mSavedWindowRect);

	// since graphics device does not cache the factory, create one again... and check feature support
	Microsoft::WRL::ComPtr<IDXGIFactory4> factory = create_debug_factory();

	mIsTearingSupported = check_feature_support(factory.Get(), DXGI_FEATURE_PRESENT_ALLOW_TEARING);

	// grab the client size of the window, assign cached width/height, create swap chain desc and create swap chain
	RECT client_rect;
	::GetClientRect(mHwnd.get(), &client_rect);
	mWidth = static_cast<UINT>(rect_width(client_rect));
	mHeight = static_cast<UINT>(rect_height(client_rect));

	DXGI_SWAP_CHAIN_DESC1 swap_chain_desc{};
	swap_chain_desc.Width = mWidth;
	swap_chain_desc.Height = mHeight;
	swap_chain_desc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	swap_chain_desc.Stereo = FALSE;
	swap_chain_desc.SampleDesc = { 1,0 };
	swap_chain_desc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	swap_chain_desc.BufferCount = SCONST_BACK_BUFFER_COUNT;
	swap_chain_desc.Scaling = DXGI_SCALING_STRETCH;
	swap_chain_desc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
	swap_chain_desc.AlphaMode = DXGI_ALPHA_MODE_UNSPECIFIED;
	swap_chain_desc.Flags = mIsTearingSupported ? DXGI_SWAP_CHAIN_FLAG_ALLOW_TEARING : 0;

	Microsoft::WRL::ComPtr<IDXGISwapChain1> swapchain1;
	execute_and_test_hresult(
		factory->CreateSwapChainForHwnd(
			mQueue->get_queue(),
			mHwnd.get(),
			&swap_chain_desc,
			nullptr,
			nullptr,
			&swapchain1
		)
	);
	execute_and_test_hresult(
		swapchain1.As(&mSwapChain)
	);

	// set the index and every resources signal value to 0
	update_current_index();

	for (auto& signal : mBufferSignals)
		signal = 0;

	// create the resource views in the descriptor heap and also cache the internal buffers for easy access
	update_back_buffers();

	mInitialised = true;
	::ShowWindow(mHwnd.get(), SW_SHOW);
}

RenderWindow::~RenderWindow()
{
	if (mQueue && mQueue->get_queue()) // not 100% sure. it should never ever happen to begin with but ye c++
		mQueue->flush();
}

CommandList RenderWindow::get_command_list()
{
	return mQueue->acquire_command_list();
}

void RenderWindow::begin()
{
	auto command_list = get_command_list();

	D3D12_RESOURCE_BARRIER barrier = easy::resource_barrier_transition(
		get_buffer(),
		D3D12_RESOURCE_STATE_PRESENT,
		D3D12_RESOURCE_STATE_RENDER_TARGET
	);
	command_list.command_list->ResourceBarrier(1, &barrier);

	command_list.command_list->ClearRenderTargetView(get_buffer_desc(), mClearColor.data(), 0, nullptr);

	mQueue->execute(std::move(command_list));
}

void RenderWindow::submit_work(CommandList&& list)
{
	mQueue->execute(std::move(list)); 
}

void RenderWindow::present()
{
	auto command_list = get_command_list();

	D3D12_RESOURCE_BARRIER barrier = easy::resource_barrier_transition(
		get_buffer(),
		D3D12_RESOURCE_STATE_RENDER_TARGET,
		D3D12_RESOURCE_STATE_PRESENT
	);

	command_list.command_list->ResourceBarrier(1, &barrier);

	set_current_buffer_signal(
		mQueue->execute(std::move(command_list))
	);

	// determine sync interval and flags
	UINT syncInterval = SCONST_IS_VSYNC ? 1 : 0;
	UINT presentFlags = (mIsTearingSupported && !syncInterval) ? DXGI_PRESENT_ALLOW_TEARING : 0;

	// present the current buffer. swap chain will internally change the current writable buffer index
	execute_and_test_hresult(
		mSwapChain->Present(syncInterval, presentFlags)
	);

	update_current_index();

	mQueue->wait_until_completion(
		current_buffer_signal()
	);
}

void RenderWindow::resize(UINT client_width, UINT client_height)
{
	// flush first
	mQueue->flush();

	// Any references to the back buffers must be released
	// before the swap chain can be resized.
	for (int i = 0; i < SCONST_BACK_BUFFER_COUNT; i++)
	{
		mBuffers[i].Reset();
		mBufferSignals[i] = current_buffer_signal();
	}
	// reset swap chains back buffers
	DXGI_SWAP_CHAIN_DESC scDesc = {};
	execute_and_test_hresult(
		mSwapChain->GetDesc(&scDesc)
	);
	execute_and_test_hresult(
		mSwapChain->ResizeBuffers(
			SCONST_BACK_BUFFER_COUNT,
			client_width,
			client_height,
			scDesc.BufferDesc.Format,
			scDesc.Flags
		));

	// set size handles
	mWidth = client_width;
	mHeight = client_height;

	// reset current index
	update_current_index();

	// update back buffer handles
	update_back_buffers();
}

void RenderWindow::toggle_fullscreen(bool mode)
{
	if (mIsFullscreen == mode)
		return;

	HWND hwnd = mHwnd.get();
	if (mode)
	{
		// cache windowed size
		::GetWindowRect(hwnd, &mSavedWindowRect);

		// change the window style attribute of the window to none, removing all decoration
		::SetWindowLongPtrW(hwnd, GWL_STYLE, 0ull);

		// query the name of the nearest display monitor and set fullscreen to the dominant one (if multi monitor)
		HMONITOR hMonitor = ::MonitorFromWindow(hwnd, MONITOR_DEFAULTTONEAREST);
		MONITORINFOEX monitorinfo = {};
		monitorinfo.cbSize = sizeof(MONITORINFOEX);
		::GetMonitorInfo(hMonitor, &monitorinfo);

		// set the position of the window and make the window top-most
		int x, y, w, h;
		x = monitorinfo.rcMonitor.left;
		y = monitorinfo.rcMonitor.top;
		w = monitorinfo.rcMonitor.right - monitorinfo.rcMonitor.left;
		h = monitorinfo.rcMonitor.bottom - monitorinfo.rcMonitor.top;
		::SetWindowPos(hwnd, HWND_TOP, x, y, w, h, SWP_FRAMECHANGED | SWP_NOACTIVATE | SWP_SHOWWINDOW);

		// set bool fullscreen
		mIsFullscreen = true;
	}
	else
	{
		// turn back on all the decor
		::SetWindowLongPtrW(hwnd, GWL_STYLE, SCONST_WINDOW_STYLE);

		// set the pos of the window to old pos
		int x, y, w, h;
		x = mSavedWindowRect.left;
		y = mSavedWindowRect.top;
		w = mSavedWindowRect.right - mSavedWindowRect.left;
		h = mSavedWindowRect.bottom - mSavedWindowRect.top;
		::SetWindowPos(hwnd, HWND_NOTOPMOST, x, y, w, h, SWP_FRAMECHANGED | SWP_NOACTIVATE | SWP_SHOWWINDOW);

		// set bool windowed
		mIsFullscreen = false;
	}
}

ID3D12Resource* RenderWindow::get_buffer() const
{
	return mBuffers[mBufferIndex].Get();
}

bool RenderWindow::create_hwnd(const RENDER_WINDOW_DESC& desc)
{
	WNDCLASSEX register_desc = {};
	register_desc.cbSize = sizeof(WNDCLASSEX);
	register_desc.lpszClassName = L"DX12RenderWindow";
	register_desc.lpfnWndProc = RenderWindow::wnd_proc;
	register_desc.hInstance = desc.hInstance;
	register_desc.style = CS_HREDRAW | CS_VREDRAW;
	register_desc.hIcon = desc.hIcon;
	register_desc.hIconSm = desc.hIconSm;
	register_desc.hCursor = desc.hCursor;
	register_desc.hbrBackground = nullptr;
	register_desc.lpszMenuName = nullptr;
	register_desc.cbClsExtra = 0;
	register_desc.cbWndExtra = 0;
	::RegisterClassExW(&register_desc);


	RECT window_rect{ static_cast<LONG>(desc.x), static_cast<LONG>(desc.y), static_cast<LONG>(desc.x + desc.width), static_cast<LONG>(desc.y + desc.height) };
	::AdjustWindowRect(&window_rect, SCONST_WINDOW_STYLE, FALSE);

	const int win_x = static_cast<int>(std::max<LONG>(window_rect.left, 0));
	const int win_y = static_cast<int>(std::max<LONG>(window_rect.top, 0));
	const int win_w = static_cast<int>(rect_width(window_rect));
	const int win_h = static_cast<int>(rect_height(window_rect));

	mHwnd.reset(
		CreateWindowExW(
			NULL,
			register_desc.lpszClassName,
			desc.window_name.c_str(),
			SCONST_WINDOW_STYLE,
			win_x,
			win_y,
			win_w,
			win_h,
			NULL,
			NULL,
			desc.hInstance,
			this
		)
	);

	return mHwnd != nullptr;
}

LRESULT RenderWindow::on_message(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	if (mInitialised) {
		switch (uMsg)
		{
		case WM_DESTROY:
			PostQuitMessage(0);
			mState.system().exit();
			break;

		case WM_SIZE:

			if (wParam != SIZE_MINIMIZED)
			{
				mState.window().resize(LOWORD(lParam), HIWORD(lParam));
			}
			break;

		case WM_SYSKEYDOWN:
		case WM_KEYDOWN:
			mState.keyboard().set_down(wParam);
			break;

		case WM_SYSKEYUP:
		case WM_KEYUP:
			mState.keyboard().set_up(wParam);
			break;

		case WM_LBUTTONDOWN:
			mState.mouse().button().set_down(MouseButtonType::Left);
			break;
		case WM_LBUTTONUP:
			mState.mouse().button().set_up(MouseButtonType::Left);
			break;
		case WM_RBUTTONDOWN:
			mState.mouse().button().set_down(MouseButtonType::Right);
			break;
		case WM_RBUTTONUP:
			mState.mouse().button().set_up(MouseButtonType::Right);
			break;
		case WM_MBUTTONDOWN:
			mState.mouse().button().set_down(MouseButtonType::Middle);
			break;
		case WM_MBUTTONUP:
			mState.mouse().button().set_up(MouseButtonType::Middle);
			break;
			//case WM_XBUTTONDOWN:
			//case WM_XBUTTONUP:
		case WM_MOUSEWHEEL:
			mState.mouse().wheel().set(GET_WHEEL_DELTA_WPARAM(wParam), WHEEL_DELTA);
			break;
		case WM_MOUSEMOVE:
		{
			int nx = ((int)(short)LOWORD(lParam));
			int ny = ((int)(short)HIWORD(lParam));
			int cx = mState.mouse().position().x();
			int cy = mState.mouse().position().y();

			if (nx != cx || ny != cy) // because windows can generate WM_MOUSEMOVE even when mouse seems stationary
				mState.mouse().hover().reset();
			mState.mouse().position().set(nx, ny);
		}
		break;

		default:
			break; // default breaks switch and goes to DefWindowProc
		}
	}
	return DefWindowProcW(hwnd, uMsg, wParam, lParam);
}

void RenderWindow::update_back_buffers()
{
	D3D12_CPU_DESCRIPTOR_HANDLE heapPos = mBufferViews.descriptor_begin();
	const UINT stride = mBufferViews.stride();

	for (UINT i = 0; i < SCONST_BACK_BUFFER_COUNT; i++)
	{
		Microsoft::WRL::ComPtr<ID3D12Resource> backBuffer;
		mSwapChain->GetBuffer(i, IID_PPV_ARGS(&backBuffer));

		mDevice->CreateRenderTargetView(backBuffer.Get(), nullptr, heapPos);

		mBuffers[i] = std::move(backBuffer);

		heapPos.ptr += stride;
	}
}

void RenderWindow::update_current_index()
{ 
	mBufferIndex = mSwapChain->GetCurrentBackBufferIndex(); 
}