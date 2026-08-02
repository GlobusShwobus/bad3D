#include "DX12RenderWindow.h"

#include "Utils.h"

DX12RenderWindow::DX12RenderWindow(
	const WINDOW_CREATE_DESC& desc,
	ObserverPtr<IDXGIFactory4> factory,
	ObserverPtr<ID3D12Device4> device,
	ObserverPtr<DX12CommandQueue> command_queue,
	UINT number_of_buffers)
{
	// basic check for vital variables then try to create the window. throw if failed
	if (!factory || !device || !command_queue || number_of_buffers < 2)
		throw_error_code_translation(static_cast<DWORD>(E_POINTER));

	if (!create_window(desc))
		throw_error_code_translation( ::GetLastError() );

	// disable alt + enter because fullscreen / windowed transitions are manual
	execute_and_test_hresult(
		factory->MakeWindowAssociation(mHwnd, DXGI_MWA_NO_ALT_ENTER)
	);

	// check if tearing is supported
	const bool is_tearing_supported = check_is_tearing_supported(factory);

	// query the true size of the client window then create the description for the swap chain
	RECT client_rect;
	::GetClientRect(mHwnd, &client_rect);
	const UINT cwidth = static_cast<UINT>(rect_width(client_rect));
	const UINT cheight = static_cast<UINT>(rect_height(client_rect));

	DXGI_SWAP_CHAIN_DESC1 swap_chain_desc = {};
	swap_chain_desc.Width                 = cwidth;
	swap_chain_desc.Height                = cheight;
	swap_chain_desc.Format                = DXGI_FORMAT_R8G8B8A8_UNORM;
	swap_chain_desc.Stereo                = FALSE;
	swap_chain_desc.SampleDesc            = { 1,0 };
	swap_chain_desc.BufferUsage           = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	swap_chain_desc.BufferCount           = number_of_buffers;
	swap_chain_desc.Scaling               = DXGI_SCALING_STRETCH;
	swap_chain_desc.SwapEffect            = DXGI_SWAP_EFFECT_FLIP_DISCARD;
	swap_chain_desc.AlphaMode             = DXGI_ALPHA_MODE_UNSPECIFIED;
	swap_chain_desc.Flags                 = is_tearing_supported ? DXGI_SWAP_CHAIN_FLAG_ALLOW_TEARING : 0;

	// with the given description create swapchain one then query interface it to memnber swapchain
	DXGISwapChain1 swapchain1;
	execute_and_test_hresult(
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

	// after swap chin set variables, then create desc heap (the order matters as creating descriptions depend on valid arguments)
	mIsTearingSupported = is_tearing_supported;
	mIsVSync = true;
	mIsFullscreen = false;
	::GetWindowRect(mHwnd, &mWindowedRect);
	mWindowStyle = desc.window_style;
	mBufferData.count = number_of_buffers;
	mBufferData.current_index = mSwapChain->GetCurrentBackBufferIndex();
	mBufferData.width = cwidth;
	mBufferData.height = cheight;
	mDevice = device;
	mCommandQueue = command_queue;

	// create descriptor heap with the given desc, create the heap then create the descriptions via reset_description_info()
	D3D12_DESCRIPTOR_HEAP_DESC descriptor_heap_desc = {};
	descriptor_heap_desc.NumDescriptors = number_of_buffers;
	descriptor_heap_desc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
	descriptor_heap_desc.NodeMask = 0;
	descriptor_heap_desc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;

	mDescHeap = std::make_unique<DX12DescriptorHeap>(descriptor_heap_desc, device);

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
	UINT syncInterval = mIsVSync ? 1 : 0;
	UINT presentFlags = (mIsTearingSupported && !mIsVSync) ? DXGI_PRESENT_ALLOW_TEARING : 0;
	// present the current buffer. swap chain will internally change the current writable buffer index
	execute_and_test_hresult(
		mSwapChain->Present(syncInterval, presentFlags)
	);

	// reset the current back buffer index
	mBufferData.current_index = mSwapChain->GetCurrentBackBufferIndex();
}

void DX12RenderWindow::on_resize()
{
	// if events run before swap chain is init
	if (!mSwapChain)
		return;

	// query the true size of the client and check if the cached resource width/height are out of sync
	RECT client_rect;
	::GetClientRect(mHwnd, &client_rect);
	const UINT cwidth = static_cast<UINT>(rect_width(client_rect));
	const UINT cheight = static_cast<UINT>(rect_height(client_rect));

	if (mBufferData.width == cwidth && mBufferData.height == cheight)
		return;

	// before resizing the resources, flush the current GPU activity
	mCommandQueue->flush();

	// reset swap chains back buffers
	DXGI_SWAP_CHAIN_DESC scDesc = {};
	execute_and_test_hresult(
		mSwapChain->GetDesc(&scDesc)
	);
	execute_and_test_hresult(
		mSwapChain->ResizeBuffers(
			mBufferData.count,
			cwidth,
			cheight,
			scDesc.BufferDesc.Format,
			scDesc.Flags
		));

	// set size handles
	mBufferData.width = cwidth;
	mBufferData.height = cheight;

	// reset current index
	mBufferData.current_index = mSwapChain->GetCurrentBackBufferIndex();

	// update back buffer handles
	reset_description_info();
}
void DX12RenderWindow::on_fullscreen_transition()
{
	if (mIsFullscreen)
		set_to_windowed();
	else
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
	return get_buffer_at(mBufferData.current_index);
}

D3D12_CPU_DESCRIPTOR_HANDLE DX12RenderWindow::get_buffer_desc()const
{
	return mDescHeap->get_descriptor_handle_for(mBufferData.current_index);
}

UINT DX12RenderWindow::get_buffer_index()const
{
	return mBufferData.current_index;
}

void DX12RenderWindow::reset_description_info() const
{
	D3D12_CPU_DESCRIPTOR_HANDLE heapPos = mDescHeap->get_descriptor_handle_for(NULL);

	for (UINT i = 0; i < mBufferData.count; i++)
	{
		mDevice->CreateRenderTargetView(get_buffer_at(i).get(), nullptr, heapPos);

		heapPos.ptr += mDescHeap->desc_size();
	}
}

void DX12RenderWindow::set_to_fullscreen()
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

void DX12RenderWindow::set_to_windowed()
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

WINDOW_REGISTER_DESC DX12RenderWindow::get_class_register_info() const noexcept
{
	WINDOW_REGISTER_DESC info = {};

	info.class_name = L"DX12RenderWindow";
	info.hInstance = g_hModule;
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