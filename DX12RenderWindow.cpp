#include "DX12RenderWindow.h"

#include "Utils.h"

DX12RenderWindow::DX12RenderWindow(
	WINDOW_EX_DESC desc,
	ObserverPtr<IDXGIFactory4> factory,
	ObserverPtr<ID3D12Device2> device,
	ObserverPtr<DX12CommandQueue> command_queue,
	UINT number_of_buffers)
{
	if (!factory || !device || !command_queue || number_of_buffers < 2)
		throw_error_code_translation(static_cast<DWORD>(E_POINTER));

	// create window
	if (!create_window(desc.window_name, desc.window_style, desc.x, desc.y, desc.w, desc.h))
		throw_error_code_translation(::GetLastError());

	// query the true size of the client
	const UIRect true_client_rect = get_client_rect(mHwnd);

	// disable alt+enter
	execute_test_throw(
		factory->MakeWindowAssociation(mHwnd, DXGI_MWA_NO_ALT_ENTER)
	);

	// check if tearing is supported
	const bool is_tearing = check_is_tearing_supported(factory);

	// create the swap chain with the given desc then use IUnknown::QueryInterface to get swap chain 4 out of it
	DXGI_SWAP_CHAIN_DESC1 swap_chain_desc = {};
	swap_chain_desc.Width = true_client_rect.w;
	swap_chain_desc.Height = true_client_rect.h;
	swap_chain_desc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	swap_chain_desc.Stereo = FALSE;
	swap_chain_desc.SampleDesc = { 1,0 };
	swap_chain_desc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	swap_chain_desc.BufferCount = number_of_buffers;
	swap_chain_desc.Scaling = DXGI_SCALING_STRETCH;
	swap_chain_desc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
	swap_chain_desc.AlphaMode = DXGI_ALPHA_MODE_UNSPECIFIED;
	swap_chain_desc.Flags = is_tearing ? DXGI_SWAP_CHAIN_FLAG_ALLOW_TEARING : 0;

	DXGISwapChain1 swapchain1;
	execute_test_throw(
		factory->CreateSwapChainForHwnd(
			command_queue->get_observer().get(),
			mHwnd,
			&swap_chain_desc,
			nullptr,
			nullptr,
			&swapchain1
		)
	);

	swapchain1.As(&mSwapChain);

	// after swap chin set variables, then create desc heap
	mTearingSupported = is_tearing;
	mVSync = true;
	mFullscreen = false;
	mWindowedRect = get_window_rect(mHwnd);
	mWindowStyle = desc.window_style;
	mBufferData.mCount = number_of_buffers;
	mBufferData.mCurrentIndex = mSwapChain->GetCurrentBackBufferIndex();
	mBufferData.mWidth = true_client_rect.w;
	mBufferData.mHeight = true_client_rect.h;
	mDevice = device;
	mCommandQueue = command_queue;

	// create descriptor heap with the given desc
	D3D12_DESCRIPTOR_HEAP_DESC descriptor_heap_desc = {};
	descriptor_heap_desc.NumDescriptors = number_of_buffers;
	descriptor_heap_desc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
	descriptor_heap_desc.NodeMask = 0;
	descriptor_heap_desc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;

	// create the descriptor heap
	mDescHeap = std::make_unique<DX12DescriptorHeap>(descriptor_heap_desc, device);

	// set descriptions
	reset_description_info();

	// show
	::ShowWindow(mHwnd, SW_SHOW);
}

DX12RenderWindow::~DX12RenderWindow()
{
	// call the window destruction
	destroy();
}

void DX12RenderWindow::present()
{
	// determine sync interval and flags
	UINT syncInterval = mVSync ? 1 : 0;
	UINT presentFlags = (mTearingSupported && !mVSync) ? DXGI_PRESENT_ALLOW_TEARING : 0;
	// present the current buffer. swap chain will internally change the current writable buffer index
	execute_test_throw(
		mSwapChain->Present(syncInterval, presentFlags)
	);

	// reset the current back buffer index
	mBufferData.mCurrentIndex = mSwapChain->GetCurrentBackBufferIndex();
}

void DX12RenderWindow::on_resize()
{
	// if events run before swap chain is init
	if (!mSwapChain)
		return;

	// query the true size of the client and check if the cached resource width/height are out of sync
	const UIRect rect = get_client_rect(mHwnd);
	if (mBufferData.mWidth == rect.w && mBufferData.mHeight == rect.h)
		return;

	// before resizing the resources, flush the current GPU activity
	mCommandQueue->flush();

	// reset swap chains back buffers
	DXGI_SWAP_CHAIN_DESC scDesc = {};
	execute_test_throw(
		mSwapChain->GetDesc(&scDesc)
	);
	execute_test_throw(
		mSwapChain->ResizeBuffers(
			mBufferData.mCount,
			rect.w,
			rect.h,
			scDesc.BufferDesc.Format,
			scDesc.Flags
		));

	// set size handles
	mBufferData.mWidth = rect.w;
	mBufferData.mHeight = rect.h;

	// reset current index
	mBufferData.mCurrentIndex = mSwapChain->GetCurrentBackBufferIndex();

	// update back buffer handles
	reset_description_info();
}
void DX12RenderWindow::on_fullscreen_transition()
{
	if (mFullscreen)
		set_to_windowed();
	else
		set_to_fullscreen();
}
void DX12RenderWindow::reconfigure(WINDOW_EX_DESC desc)
{
	// if currently fullscreen, drop back to windowed first — otherwise we'd be
	// resizing the fullscreen borderless rect instead of the real windowed one,
	// and mWindowedRect would end up wrong for the next set_to_windowed() call.
	const bool current_mode = mFullscreen;
	if (mFullscreen)
		set_to_windowed();

	// update the cached windowed geometry so a later set_to_windowed()/alt-tab-back
	// restores to the NEW size/position, not the previous demo's.
	mWindowedRect.x = desc.x;
	mWindowedRect.y = desc.y;
	mWindowedRect.w = desc.w;
	mWindowedRect.h = desc.h;

	// retitle
	::SetWindowTextW(mHwnd, desc.window_name);

	// repos + resize
	::SetWindowPos(
		mHwnd, nullptr,
		desc.x, desc.y, desc.w, desc.h,
		SWP_NOZORDER | SWP_NOACTIVATE
	);

	// re-enter fullscreen if the new demo wants to start there
	if (current_mode != mFullscreen)
		set_to_fullscreen();
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
	return get_buffer_at(mBufferData.mCurrentIndex);
}

D3D12_CPU_DESCRIPTOR_HANDLE DX12RenderWindow::get_buffer_desc()const
{
	return mDescHeap->get_descriptor_handle_for(mBufferData.mCurrentIndex);
}

UINT DX12RenderWindow::get_buffer_index()const
{
	return mBufferData.mCurrentIndex;
}

void DX12RenderWindow::reset_description_info() const
{
	D3D12_CPU_DESCRIPTOR_HANDLE heapPos = mDescHeap->get_descriptor_handle_for(NULL);

	for (UINT i = 0; i < mBufferData.mCount; i++)
	{
		mDevice->CreateRenderTargetView(get_buffer_at(i).get(), nullptr, heapPos);

		heapPos.ptr += mDescHeap->desc_size();
	}
}

void DX12RenderWindow::set_to_fullscreen()
{
	// cache windowed size
	mWindowedRect = get_window_rect(mHwnd);

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
	mFullscreen = true;
}

void DX12RenderWindow::set_to_windowed()
{
	// turn back on all the decor
	::SetWindowLongPtrW(mHwnd, GWL_STYLE, mWindowStyle);

	// set the pos of the window to old pos
	::SetWindowPos(mHwnd, HWND_NOTOPMOST, mWindowedRect.x, mWindowedRect.y, mWindowedRect.w, mWindowedRect.h, SWP_FRAMECHANGED | SWP_NOACTIVATE | SWP_SHOWWINDOW);

	// set bool windowed
	mFullscreen = false;
}

IWin32Window::WindowClassRegister DX12RenderWindow::get_class_register_info() const noexcept
{
	WindowClassRegister info = {};

	info.class_name = L"DX12RenderWindow";
	info.module_ = g_hModule;
	info.class_style = CS_HREDRAW | CS_VREDRAW;
	info.hIcon = nullptr;
	info.hIconSm = nullptr;
	info.hCursor = nullptr;
	info.hbrBackground = nullptr;
	info.lpszMenuName = nullptr;
	info.cbClsExtra = NULL;
	info.cbWndExtra = NULL;

	return info;
}