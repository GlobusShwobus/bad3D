#include "RenderWindow.h"
#include "Utils.h"

RenderWindow::RenderWindow(
	ViewPtr<HWND__> hwnd,
	ViewPtr<IDXGIFactory4> factory,
	ViewPtr<ID3D12Device4> device,
	ViewPtr<ID3D12CommandQueue> command_queue,
	DWORD window_style
	)
{
	assert(hwnd && "window nullptr");
	assert(factory && "factory nullptr");
	assert(device && "device nullptr");
	assert(command_queue && "command_queue nullptr");
	// desc heap for the buffers
	mDescHeap.init(device, D3D12_DESCRIPTOR_HEAP_TYPE_RTV, back_buffer_count);

	// check if tearing is supported
	BOOL allow_tearing = FALSE;
	Microsoft::WRL::ComPtr<IDXGIFactory5> factory5;
	if (SUCCEEDED(factory->QueryInterface(IID_PPV_ARGS(&factory5))))
		if (FAILED(factory5->CheckFeatureSupport(DXGI_FEATURE_PRESENT_ALLOW_TEARING, &allow_tearing, sizeof(allow_tearing))))
			allow_tearing = FALSE;
	mIsTearingSupported = allow_tearing == TRUE;

	// query the true size of the client window then create the description for the swap chain
	RECT client_rect;
	::GetClientRect(hwnd.get(), &client_rect);
	const UINT width = static_cast<UINT>(client_rect.right - client_rect.left);
	const UINT height = static_cast<UINT>(client_rect.bottom - client_rect.top);

	DXGI_SWAP_CHAIN_DESC1 swap_chain_desc = {};
	swap_chain_desc.Width                 = width;
	swap_chain_desc.Height                = height;
	swap_chain_desc.Format                = DXGI_FORMAT_R8G8B8A8_UNORM;
	swap_chain_desc.Stereo                = FALSE;
	swap_chain_desc.SampleDesc            = { 1,0 };
	swap_chain_desc.BufferUsage           = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	swap_chain_desc.BufferCount           = back_buffer_count;
	swap_chain_desc.Scaling               = DXGI_SCALING_STRETCH;
	swap_chain_desc.SwapEffect            = DXGI_SWAP_EFFECT_FLIP_DISCARD;
	swap_chain_desc.AlphaMode             = DXGI_ALPHA_MODE_UNSPECIFIED;
	swap_chain_desc.Flags                 = mIsTearingSupported ? DXGI_SWAP_CHAIN_FLAG_ALLOW_TEARING : 0;

	// with the given description create swapchain one then query interface it to memnber swapchain
	Microsoft::WRL::ComPtr<IDXGISwapChain1> swapchain1;
	execute_and_test_hresult(
		factory->CreateSwapChainForHwnd(
			command_queue.get(),
			hwnd.get(),
			&swap_chain_desc,
			nullptr,
			nullptr,
			&swapchain1
		)
	);

	swapchain1.As(&mSwapChain);

	// after swap chin set variables, then create desc heap (the order matters as creating descriptions depend on valid arguments)
	mIsVSync = true;
	mScreenToggle.is_fullscreen = false;
	::GetWindowRect(mHwnd.get(), &mScreenToggle.window_rect);
	mScreenToggle.window_style = window_style;
	mCurrentBBIndex = mSwapChain->GetCurrentBackBufferIndex();
	mBBWidth = width;
	mBBHeight = height;
	mDevice = device;
	mHwnd = hwnd;

	// set the descriptors in the descriptor heap
	reset_description_info();
}

void RenderWindow::present_to_display()
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

void RenderWindow::resize( UINT client_width,  UINT client_height)
{
	// reset swap chains back buffers
	DXGI_SWAP_CHAIN_DESC scDesc = {};
	execute_and_test_hresult(
		mSwapChain->GetDesc(&scDesc)
	);
	execute_and_test_hresult(
		mSwapChain->ResizeBuffers(
			back_buffer_count,
			client_width,
			client_height,
			scDesc.BufferDesc.Format,
			scDesc.Flags
		));

	// set size handles
	mBBWidth = client_width;
	mBBHeight = client_height;

	// reset current index
	mCurrentBBIndex = mSwapChain->GetCurrentBackBufferIndex();

	// update back buffer handles
	reset_description_info();
}

void RenderWindow::toggle_fullscreen(bool fullscreen)
{
	mScreenToggle.toggle_window_to(mHwnd.get(), fullscreen);
}

ViewPtr<ID3D12Resource> RenderWindow::get_buffer_at(UINT index) const
{
	// comptr forces ref counting but pass but just the view. fatal errors are fatal erros which shouldn't happen to begin with
	Microsoft::WRL::ComPtr<ID3D12Resource> buffer;
	mSwapChain->GetBuffer(index, IID_PPV_ARGS(&buffer));
	return buffer.Get();
}

ViewPtr<ID3D12Resource> RenderWindow::get_buffer() const
{
	return get_buffer_at(mCurrentBBIndex);
}

D3D12_CPU_DESCRIPTOR_HANDLE RenderWindow::get_buffer_desc()const
{
	return mDescHeap.get_descriptor_handle_for(mCurrentBBIndex);
}

RECT RenderWindow::get_client_rect() const
{
	RECT r;
	::GetClientRect(mHwnd.get(), &r);
	return r;
}

void RenderWindow::reset_description_info() const
{
	D3D12_CPU_DESCRIPTOR_HANDLE heapPos = mDescHeap.get_descriptor_handle_for(NULL);

	for (UINT i = 0; i < back_buffer_count; i++)
	{
		mDevice->CreateRenderTargetView(get_buffer_at(i).get(), nullptr, heapPos);

		heapPos.ptr += mDescHeap.mDescriptorSize;
	}
}

void RenderWindow::DescHeap::init(ViewPtr<ID3D12Device4> device, D3D12_DESCRIPTOR_HEAP_TYPE type, UINT count)
{
	D3D12_DESCRIPTOR_HEAP_DESC descriptor_heap_desc = {};
	descriptor_heap_desc.NumDescriptors = count;
	descriptor_heap_desc.Type = type;
	descriptor_heap_desc.NodeMask = 0;
	descriptor_heap_desc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;

	execute_and_test_hresult(
		device->CreateDescriptorHeap(&descriptor_heap_desc, IID_PPV_ARGS(&mDescriptorHeap))
	);

	mDescriptorSize = device->GetDescriptorHandleIncrementSize(type);
}

D3D12_CPU_DESCRIPTOR_HANDLE RenderWindow::DescHeap::get_descriptor_handle_for(SIZE_T index) const noexcept
{
	assert(mDescriptorHeap && "nullptr");

	D3D12_CPU_DESCRIPTOR_HANDLE handle = { 0 };
	// pointer arithmetic, offset from begin to index times size in bytes
	handle.ptr = mDescriptorHeap->GetCPUDescriptorHandleForHeapStart().ptr
		+ index * mDescriptorSize;
	return handle;
}

void RenderWindow::ScreenToggle::toggle_window_to( HWND hwnd, bool mode )
{
	if (is_fullscreen == mode)
		return;

	if (mode)
	{
		// cache windowed size
		::GetWindowRect(hwnd, &window_rect);

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
		is_fullscreen = true;
	}
	else
	{
		// turn back on all the decor
		::SetWindowLongPtrW(hwnd, GWL_STYLE, window_style);

		// set the pos of the window to old pos
		int x, y, w, h;
		x = window_rect.left;
		y = window_rect.top;
		w = window_rect.right - window_rect.left;
		h = window_rect.bottom - window_rect.top;
		::SetWindowPos(hwnd, HWND_NOTOPMOST, x, y, w, h, SWP_FRAMECHANGED | SWP_NOACTIVATE | SWP_SHOWWINDOW);

		// set bool windowed
		is_fullscreen = false;
	}
}
