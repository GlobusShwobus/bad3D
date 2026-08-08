#include "DX12RenderWindow.h"

#include "Utils.h"

DX12RenderWindow::DX12RenderWindow(
	const std::wstring& title, UINT x, UINT y, UINT client_width, UINT client_height, DWORD window_style,
	ObserverPtr<IDXGIFactory4> factory,
	ObserverPtr<ID3D12Device4> device,
	ObserverPtr<ID3D12CommandQueue> command_queue,
	ObserverPtr<IWindowEventListener> listener
	)
	:mDescHeap(device, back_buffer_count, D3D12_DESCRIPTOR_HEAP_TYPE_RTV)
{
	// adjust client size to window size and create the window
	RECT window_rect{ x,y, client_width, client_height };
	::AdjustWindowRect(&window_rect, window_style, FALSE);

	if (!create_window(title, window_rect.left, window_rect.top, rect_width(window_rect), rect_height(window_rect), window_style))
		throw_error_code_translation( ::GetLastError() );

	// disable alt + enter because fullscreen / windowed transitions are manual
	execute_and_test_hresult(
		factory->MakeWindowAssociation(mHwnd, DXGI_MWA_NO_ALT_ENTER)
	);

	// bind listener to do message loop
	execute_and_test_hresult(
		bind_event_listener(listener)
	);

	// check if tearing is supported
	const bool is_tearing_supported = check_is_tearing_supported(factory);

	// query the true size of the client window then create the description for the swap chain
	UINT cwidth, cheight;
	get_client_size(cwidth,cheight);

	DXGI_SWAP_CHAIN_DESC1 swap_chain_desc = {};
	swap_chain_desc.Width                 = cwidth;
	swap_chain_desc.Height                = cheight;
	swap_chain_desc.Format                = DXGI_FORMAT_R8G8B8A8_UNORM;
	swap_chain_desc.Stereo                = FALSE;
	swap_chain_desc.SampleDesc            = { 1,0 };
	swap_chain_desc.BufferUsage           = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	swap_chain_desc.BufferCount           = back_buffer_count;
	swap_chain_desc.Scaling               = DXGI_SCALING_STRETCH;
	swap_chain_desc.SwapEffect            = DXGI_SWAP_EFFECT_FLIP_DISCARD;
	swap_chain_desc.AlphaMode             = DXGI_ALPHA_MODE_UNSPECIFIED;
	swap_chain_desc.Flags                 = is_tearing_supported ? DXGI_SWAP_CHAIN_FLAG_ALLOW_TEARING : 0;

	// with the given description create swapchain one then query interface it to memnber swapchain
	DXGISwapChain1 swapchain1;
	execute_and_test_hresult(
		factory->CreateSwapChainForHwnd(
			command_queue.get(),
			mHwnd,
			&swap_chain_desc,
			nullptr,
			nullptr,
			&swapchain1
		)
	);

	swapchain1.As(&mSwapChain);

	// after swap chin set variables, then create desc heap (the order matters as creating descriptions depend on valid arguments)
	mIsTearingSupported = is_tearing_supported;
	mIsVSync = true;
	mIsFullscreen = false;
	::GetWindowRect(mHwnd, &mWindowedRect);
	mWindowStyle = window_style;
	mCurrentBBIndex = mSwapChain->GetCurrentBackBufferIndex();
	mBBWidth = cwidth;
	mBBHeight = cheight;
	mDevice = device;

	// set the descriptors in the descriptor heap
	reset_description_info();

	// show
	::ShowWindow(mHwnd, SW_SHOW);
}

DX12RenderWindow::~DX12RenderWindow()
{
	// call the window destruction
	destroy();
}

void DX12RenderWindow::present_to_display()
{
	// determine sync interval and flags
	UINT syncInterval = mIsVSync ? 1 : 0;
	UINT presentFlags = (mIsTearingSupported && !mIsVSync) ? DXGI_PRESENT_ALLOW_TEARING : 0;
	// present the current buffer. swap chain will internally change the current writable buffer index
	execute_and_test_hresult(
		mSwapChain->Present(syncInterval, presentFlags)
	);

	// reset the current back buffer index
	mCurrentBBIndex = mSwapChain->GetCurrentBackBufferIndex();
}

void DX12RenderWindow::resize(const UINT cwidth, const UINT cheight)
{
	// before resizing the resources, flush the current GPU activity
	//mCommandQueue->flush();

	// reset swap chains back buffers
	DXGI_SWAP_CHAIN_DESC scDesc = {};
	execute_and_test_hresult(
		mSwapChain->GetDesc(&scDesc)
	);
	execute_and_test_hresult(
		mSwapChain->ResizeBuffers(
			back_buffer_count,
			cwidth,
			cheight,
			scDesc.BufferDesc.Format,
			scDesc.Flags
		));

	// set size handles
	mBBWidth = cwidth;
	mBBHeight = cheight;

	// reset current index
	mCurrentBBIndex = mSwapChain->GetCurrentBackBufferIndex();

	// update back buffer handles
	reset_description_info();
}
void DX12RenderWindow::toggle_fullscreen(bool fullscreen)
{
	if (mIsFullscreen == fullscreen)
		return;

	if (fullscreen)
	{
		// cache windowed size
		::GetWindowRect(mHwnd, &mWindowedRect);

		// remove all decoration
		const UINT windowStyle = 0;

		// change the window style attribute of the window with the given style above
		::SetWindowLongPtrW(mHwnd, GWL_STYLE, windowStyle);

		// query the name of the nearest display monitor and set fullscreen to the dominant one (if multi monitor)
		HMONITOR hMonitor = ::MonitorFromWindow(mHwnd, MONITOR_DEFAULTTONEAREST);
		MONITORINFOEX monitorinfo = {};
		monitorinfo.cbSize = sizeof(MONITORINFOEX);
		::GetMonitorInfo(hMonitor, &monitorinfo);

		// set the position of the window and make the window top-most
		int x, y, w, h;
		x = monitorinfo.rcMonitor.left;
		y = monitorinfo.rcMonitor.top;
		w = monitorinfo.rcMonitor.right - monitorinfo.rcMonitor.left;
		h = monitorinfo.rcMonitor.bottom - monitorinfo.rcMonitor.top;
		::SetWindowPos(mHwnd, HWND_TOP, x, y, w, h, SWP_FRAMECHANGED | SWP_NOACTIVATE | SWP_SHOWWINDOW);

		// set bool fullscreen
		mIsFullscreen = true;
	}
	else
	{
		// turn back on all the decor
		::SetWindowLongPtrW(mHwnd, GWL_STYLE, mWindowStyle);

		// set the pos of the window to old pos
		int x, y, w, h;
		x = mWindowedRect.left;
		y = mWindowedRect.top;
		w = mWindowedRect.right - mWindowedRect.left;
		h = mWindowedRect.bottom - mWindowedRect.top;
		::SetWindowPos(mHwnd, HWND_NOTOPMOST, x, y, w, h, SWP_FRAMECHANGED | SWP_NOACTIVATE | SWP_SHOWWINDOW);

		// set bool windowed
		mIsFullscreen = false;
	}
}

ObserverPtr<ID3D12Resource> DX12RenderWindow::get_buffer_at(UINT index) const
{
	// comptr forces ref counting but pass but just the view. fatal errors are fatal erros which shouldn't happen to begin with
	D3D12Resource buffer;
	mSwapChain->GetBuffer(index, IID_PPV_ARGS(&buffer));
	return buffer.Get();
}

ObserverPtr<ID3D12Resource> DX12RenderWindow::get_buffer() const
{
	return get_buffer_at(mCurrentBBIndex);
}

D3D12_CPU_DESCRIPTOR_HANDLE DX12RenderWindow::get_buffer_desc()const
{
	return mDescHeap.get_descriptor_handle_for(mCurrentBBIndex);
}

UINT DX12RenderWindow::get_buffer_index()const
{
	return mCurrentBBIndex;
}

void DX12RenderWindow::get_client_size(UINT& width, UINT& height) const
{
	RECT client_rect;
	::GetClientRect(mHwnd, &client_rect);
	width = static_cast<UINT>(rect_width(client_rect));
	height = static_cast<UINT>(rect_height(client_rect));
}

void DX12RenderWindow::reset_description_info() const
{
	D3D12_CPU_DESCRIPTOR_HANDLE heapPos = mDescHeap.get_descriptor_handle_for(NULL);
	auto desc_size = mDescHeap.desc_size();
	for (UINT i = 0; i < back_buffer_count; i++)
	{
		mDevice->CreateRenderTargetView(get_buffer_at(i).get(), nullptr, heapPos);

		heapPos.ptr += desc_size;
	}
}

WNDCLASSEX DX12RenderWindow::get_class_register_info() const noexcept
{
	WNDCLASSEX info = {};

	info.lpszClassName = L"DX12RenderWindow";
	info.hInstance = g_hModule;
	info.style = CS_HREDRAW | CS_VREDRAW;
	info.hIcon = nullptr;
	info.hIconSm = nullptr;
	info.hCursor = nullptr;
	info.hbrBackground = nullptr;
	info.lpszMenuName = nullptr;
	info.cbClsExtra = NULL;
	info.cbWndExtra = NULL;

	return info;
}