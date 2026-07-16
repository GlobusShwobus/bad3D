#pragma once

#include "GPU_CORE.h"
#include "Utils.h"

#if defined(min)
#undef min
#endif

#if defined(max)
#undef max
#endif

#include "DX12CommandQueue.h"

static LRESULT CALLBACK reserved_wnd_proc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

struct GFXSettings
{
	bool vsync = true;
	bool tearing_supported = false;
	bool fullscreen = false;

	uint32_t client_width;
	uint32_t client_height;
};

class DX12Window
{
	// *this should never move or be copied. A wrapper around HWND is not very pretty (kind of fragile and not correct). but
	// it allows for correct *this destructor sequence.
	class MSWindow {
	public:
		MSWindow() :mHwnd(nullptr) {}
		explicit MSWindow(HWND hwnd)
			:mHwnd(hwnd)
		{
			if (!mHwnd)
				throw std::runtime_error("nullptr");
		}
		MSWindow(const MSWindow&) = delete;
		MSWindow& operator=(const MSWindow&) = delete;

		MSWindow(MSWindow&& other) noexcept
			: mHwnd(other.mHwnd)
		{
			other.mHwnd = nullptr;
		}

		MSWindow& operator=(MSWindow&& other) noexcept
		{
			if (this != &other)
			{
				if (mHwnd)
					DestroyWindow(mHwnd);

				mHwnd = other.mHwnd;
				other.mHwnd = nullptr;
			}
			return *this;
		}

		~MSWindow()noexcept
		{
			if (mHwnd)
				::DestroyWindow(mHwnd);
		}
		HWND get()const { return mHwnd; }
	private:
		HWND mHwnd = nullptr;
	};

	using WNDCLASSEX_DESC = WNDCLASSEX;

	// add window proc as a friend to allow bullshit
	friend LRESULT CALLBACK reserved_wnd_proc(HWND, UINT, WPARAM, LPARAM);

	// the number of back buffers for the swap chain aka surfaces aka drawable frames
	static constexpr std::size_t NUMBER_OF_BUFFERS = 3;

	// color for clear screen
	static constexpr FLOAT G_CLEAR_SCREEN_COL[4] = {0.4f,0.6f,0.9f,1.0f};

	// windows advanced rasterization protocol
	static constexpr bool G_IS_WARP = false;

	// doesnt make sense for these 
	DX12Window(const DX12Window&) = delete;
	DX12Window& operator=(const DX12Window&) = delete;
	DX12Window(DX12Window&&) = delete;
	DX12Window& operator=(DX12Window&&) = delete;

public:

	DX12Window(HINSTANCE module, const wchar_t* title, uint32_t width, uint32_t height, DWORD window_style)
	{
		const wchar_t window_class_name[] = L"Graphics window";
		const D3D12_COMMAND_LIST_TYPE command_list_type = D3D12_COMMAND_LIST_TYPE_DIRECT;

		// window description
		WNDCLASSEX_DESC window_desc = {};
		window_desc.cbSize = sizeof(window_desc);
		window_desc.lpfnWndProc = reserved_wnd_proc;
		window_desc.lpszClassName = window_class_name;
		window_desc.style = CS_HREDRAW | CS_VREDRAW;
		window_desc.hInstance = module;
		window_desc.cbClsExtra = 0;
		window_desc.cbWndExtra = 0;
		window_desc.hIcon = nullptr;
		window_desc.hCursor = nullptr;
		window_desc.hbrBackground = nullptr;
		window_desc.lpszMenuName = nullptr;
		window_desc.hIconSm = nullptr;

		// command queue description

		// swap chain description
		DXGI_SWAP_CHAIN_DESC1 swap_chain_desc = {};
		swap_chain_desc.Width = width;
		swap_chain_desc.Height = height;
		swap_chain_desc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
		swap_chain_desc.Stereo = FALSE;
		swap_chain_desc.SampleDesc = { 1,0 };
		swap_chain_desc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
		swap_chain_desc.BufferCount = NUMBER_OF_BUFFERS;
		swap_chain_desc.Scaling = DXGI_SCALING_STRETCH;
		swap_chain_desc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
		swap_chain_desc.AlphaMode = DXGI_ALPHA_MODE_UNSPECIFIED;
		 
		// descriptor heap description
		D3D12_DESCRIPTOR_HEAP_DESC descriptor_heap_desc = {};
		descriptor_heap_desc.NumDescriptors = NUMBER_OF_BUFFERS;
		descriptor_heap_desc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
		descriptor_heap_desc.NodeMask = 0; // single adapter system
		descriptor_heap_desc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;

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


		// register the window class
		if (RegisterClassExW(&window_desc) == 0)
			throw_error_code_translation(GetLastError());

		// create window. the window is created as a temporary and actually assigned in window procedure on WM_NCCREATE
		int x, y, w, h;
		adjust_desired_client_and_window_size(width, height, x, y, w, h);
		HWND window = CreateWindowExW(
			NULL,                                          // extended window addons
			window_class_name,                             // registered name
			title,										   // window name
			window_style,								   // additional add ons
			x,y,w,h,                                       // window pos and size
			NULL,                                          // parent window 
			NULL,                                          // menu window
			module,										   // src module
			this                                           // for window_procedure
		);

		if (!window)
			throw_error_code_translation(GetLastError());
		mSettings.client_width = width;
		mSettings.client_height = height;

		// create DXGI factory
		DXFactory4 factory;
		UINT create_factory_flags = 0;  // only 2 values are valid: 0 or debug.
#if defined(_DEBUG)
		create_factory_flags = DXGI_CREATE_FACTORY_DEBUG;
#endif
		execute_test_throw(
			CreateDXGIFactory2(create_factory_flags, IID_PPV_ARGS(&factory))
		);

		// Disable the Alt+Enter fullscreen toggle feature. Switching to fullscreen
		// will be handled manually.
		execute_test_throw(
			factory->MakeWindowAssociation(window, DXGI_MWA_NO_ALT_ENTER)
		);

		// find a good adapter
		DXAdapter4 adapter4;
		execute_test_throw(
			find_adapter(factory, G_IS_WARP, adapter4)
		);

		// create device
		execute_test_throw(
			D3D12CreateDevice(adapter4.Get(), D3D_FEATURE_LEVEL_11_0, IID_PPV_ARGS(&mDevice))
		);

#if defined(_DEBUG)
		// set device debug info
		DXInfoQueue infoQueue;
		if (SUCCEEDED(mDevice.As(&infoQueue)))
		{
			// set break point for types
			infoQueue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_CORRUPTION, TRUE);
			infoQueue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_ERROR, TRUE);
			infoQueue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_WARNING, TRUE);

			// ignore messages
			execute_test_throw(
				infoQueue->PushStorageFilter(&info_queue_deny_filter)
			);
		}
#endif
		// create command queue
		mCommand_queue = std::make_unique<DX12CommandQueue>(mDevice, command_list_type);

		// check if display supports tearing. set local variable for later use and update the swap chain desc
		mSettings.tearing_supported = check_is_tearing_supported(factory);
		swap_chain_desc.Flags = mSettings.tearing_supported ? DXGI_SWAP_CHAIN_FLAG_ALLOW_TEARING : 0;

		// create the swap chain
		DXSwapChain1 swapchain1;
		execute_test_throw(
			factory->CreateSwapChainForHwnd(
				mCommand_queue->get_private(),	  // in directx 12 this IUnknown must be a command queue, not device
				window, 				  // the window that is associated with the swap chain
				&swap_chain_desc,				  // description
				nullptr,			  // optional fullscreen desc. do windowed fullscreen instead
				nullptr,			  // 
				&swapchain1			  // create the swapchain
			)
		);

		execute_test_throw(
			swapchain1.As(&mSwap_chain)
		);

		// set the the back buffer tracking index
		mCurrent_back_buffer_index = mSwap_chain->GetCurrentBackBufferIndex();

		// create descriptor heap and descriptor size
		execute_test_throw(
			mDevice->CreateDescriptorHeap(&descriptor_heap_desc, IID_PPV_ARGS(&mDescriptor_heap))
		);
		// cache the system descriptor size
		mDescriptor_size = mDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);

		// create RTV buffer handles and descriptors
		reset_back_buffers();

		// create command allocator

		// create command list
		mCommand_list = mCommand_queue->get_command_list();

		execute_test_throw(
			mCommand_list->Close()
		);

		// create fence

		// set is init which up until this point protecd against sytem commands
		is_init = true;

		// show window
		ShowWindow(window, SW_SHOW);
	}

	~DX12Window()
	{
		// finish any workload the GPU is doing before destruction
		// flush();
	}

	void dispatch_event_reading()
	{
		// call wndproc
		MSG msg = {};
		while (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE))
		{
			//TranslateMessage(&msg); // additional messages like WM_CHAR. checking wParam manually works too. but needs research
			DispatchMessage(&msg);
		}
	}

	void setup_frame()
	{
		// reset command allocator and list to default state before recording new commands this frame
		// auto& command_allocator = mCommand_allocators[mCurrent_back_buffer_index];
		auto& back_buffer       = mBack_buffers[mCurrent_back_buffer_index];

		//command_allocator->Reset();
		//mCommand_list->Reset(command_allocator.Get(), nullptr);
		
		// get new command list
		mCommand_list = mCommand_queue->get_command_list();

		// clear the render target
		{
			// set up a transition command for the render target to go from present state to target state
			D3D12_RESOURCE_BARRIER transition_rt = {};
			transition_rt.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
			transition_rt.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
			transition_rt.Transition.pResource = back_buffer.Get();
			transition_rt.Transition.StateBefore = D3D12_RESOURCE_STATE_PRESENT;         // this knowledge is implied and for back buffers it's fine because they're only ever in one or the other state. for other resources that get more complex logic, state tracking becomes important
			transition_rt.Transition.StateAfter = D3D12_RESOURCE_STATE_RENDER_TARGET;
			transition_rt.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;

			// add the command to the list
			mCommand_list->ResourceBarrier(1, &transition_rt);
			// index into the correct descriptor in the descriptor heap
			D3D12_CPU_DESCRIPTOR_HANDLE index = get_current_buffers_descriptor_index();
			mCommand_list->ClearRenderTargetView(index, G_CLEAR_SCREEN_COL, 0, nullptr);
		}
	}

	void present_frame()
	{
		auto& back_buffer = mBack_buffers[mCurrent_back_buffer_index];
		// set up a transition command for the render target to go from rendering state to present state
		D3D12_RESOURCE_BARRIER transition_rt = {};
		transition_rt.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
		transition_rt.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
		transition_rt.Transition.pResource = back_buffer.Get();
		transition_rt.Transition.StateBefore = D3D12_RESOURCE_STATE_RENDER_TARGET;
		transition_rt.Transition.StateAfter = D3D12_RESOURCE_STATE_PRESENT;
		transition_rt.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;

		// add the command to the list
		mCommand_list->ResourceBarrier(1, &transition_rt);

		// signal that the recording into the list is finished
		execute_test_throw(
			mCommand_list->Close()
		);
		// eventually it will be cool to have multiple list. command queue wants an array of lists by defualt. right now it's just 1 though
		// ID3D12CommandList* const command_lists[] = { mCommand_list.Get() };

		// execute list(s)
		UINT64 signal_val = mCommand_queue->execute(mCommand_list);
		//mCommand_queue->ExecuteCommandLists(_countof(command_lists), command_lists);

		// present to display via swap chain
		UINT syncInterval = mSettings.vsync ? 1 : 0;
		UINT presentFlags = (mSettings.tearing_supported && !mSettings.vsync) ? DXGI_PRESENT_ALLOW_TEARING : 0;
		execute_test_throw(
			mSwap_chain->Present(syncInterval, presentFlags)
		);

		// ask for the GPU to signal when done rendering THIS frame
		// mExpected_fence_values[mCurrent_back_buffer_index] = mFence.signal();
		
		// swap chain will change its internal back buffer index after calling present
		// the system is about to reuse a buffer. retrieve the new index of the buffer and stall the CPU in case the GPU is not yet done with it
		mCurrent_back_buffer_index = mSwap_chain->GetCurrentBackBufferIndex();

		mCommand_queue->wait(signal_val);
		//mFence.wait(mExpected_fence_values[mCurrent_back_buffer_index]);
	}

	bool is_open()const
	{
		return is_init == true;
	}

private:

	LRESULT wnd_proc(UINT uMsg, WPARAM wParam, LPARAM lParam)
	{
		if (is_init) {
			switch (uMsg)
			{
			case WM_DESTROY:
				PostQuitMessage(0);
				is_init = false;
				return 0;
			case WM_PAINT:
			{
				// PAINTSTRUCT ps;
				// BeginPaint(mWindow.get(), &ps);
				// EndPaint(mWindow.get(), &ps);
			}
			break;

			// on window resize get the width and height to the client area. alternative is width / height = LOPARAM / HIPARAM (lParam) but this gives entire size
			case WM_SIZE:

				if (wParam != SIZE_MINIMIZED)
				{
					RECT clientRect;
					GetClientRect(mWindow.get(), &clientRect);
					resize_buffers_on_WM_SIZE(clientRect.right - clientRect.left, clientRect.bottom - clientRect.top);
				}
				break;

				// full screen toggle must be a special case becasue on messing with the window, something deep in win32 gets messed up.
				// currently i think the best is to approach it as a special case, hence add the event into the main event handle and not on physical key down / up
				// default window proc will play a system notification sound
			case WM_SYSKEYDOWN:
			case WM_KEYDOWN:

				if (wParam == VK_F11)
				{
					set_FSBW_mode(!mSettings.fullscreen);
				}

				break;

			default:
				break; // default breaks switch and goes to DefWindowProc
			}


			//	// window must also update the mouse and keyboard events
			//	m_mouse.handle_mouse_messages(uMsg, wParam, lParam, m_hwnd);
			//	m_kb.handle_mouse_messages(uMsg, wParam, lParam);
			//	
			//	// set mouse capture, may be done in higher level code. will see
			//	if (m_mouse.button(Mouse::ButtonType::Left).pressed()) {
			//		SetCapture(m_hwnd);
			//	}
			//	else if (m_mouse.button(Mouse::ButtonType::Left).released())
			//	{
			//		if (GetCapture() == m_hwnd) {
			//			ReleaseCapture();
			//		}
			//	}
		}
		return DefWindowProcW(mWindow.get(), uMsg, wParam, lParam);
	}

	// this function maps / remaps backbuffers and descriptors. use this after resize or in anycase the buffers chang
	void reset_back_buffers()
	{
		D3D12_CPU_DESCRIPTOR_HANDLE heapPos(mDescriptor_heap->GetCPUDescriptorHandleForHeapStart());

		for (UINT i=0; i< NUMBER_OF_BUFFERS;i++)
		{
			// release current handle
			mBack_buffers[i].Reset();
			// reassign handle
			mSwap_chain->GetBuffer(i, IID_PPV_ARGS(&mBack_buffers[i]));
			// rewrite description in heap
			mDevice->CreateRenderTargetView(mBack_buffers[i].Get(), nullptr, heapPos);
			//increment pos
			heapPos.ptr += mDescriptor_size;
		}
	}

	// this function force waits for the moment the GPU is done with the commands. use it for resetting moments like on resize
	//	void flush()
	//	{
	//		UINT64 expected_value = mFence.signal();
	//		mFence.wait(expected_value);
	//	}

	// gets the current handle ( behaves like an index actually ) of the descriptor
	D3D12_CPU_DESCRIPTOR_HANDLE get_current_buffers_descriptor_index() const
	{
		D3D12_CPU_DESCRIPTOR_HANDLE index = { 0 };
		index.ptr += mDescriptor_heap->GetCPUDescriptorHandleForHeapStart().ptr + (mCurrent_back_buffer_index * mDescriptor_size);
		return index;
	}

	void resize_buffers_on_WM_SIZE(uint32_t width, uint32_t height)
	{
		if (mSettings.client_width != width || mSettings.client_height != height)
		{
			// a buffer nor the window can be 0 size
			mSettings.client_width = std::max(1u, width);
			mSettings.client_height = std::max(1u, height);
			
			// make sure the resources are free before reuse
			mCommand_queue->flush();

			// any references to the back buffers must be released
			// for proper bookkeeping the expected fence values are also set to whatever is the most current one 
			// because old buffers are gone therefor nothing should be expected regarding them. it's essentially a reset value, nothing more.
			for (int i = 0; i < NUMBER_OF_BUFFERS; i++)
			{
				mBack_buffers[i].Reset();
				// mExpected_fence_values[i] = mExpected_fence_values[mCurrent_back_buffer_index];
			}

			// reset swap chains back buffers
			DXGI_SWAP_CHAIN_DESC scDesc = {};
			execute_test_throw(
				mSwap_chain->GetDesc(&scDesc)
			);
			execute_test_throw(
				mSwap_chain->ResizeBuffers(
					NUMBER_OF_BUFFERS, 
					mSettings.client_width,
					mSettings.client_height,
					scDesc.BufferDesc.Format,
					scDesc.Flags
				));
			// reset current index
			mCurrent_back_buffer_index = mSwap_chain->GetCurrentBackBufferIndex();
			// reset back buffers and descriptions
			reset_back_buffers();
		}
	}

	void set_FSBW_mode(bool fullscreen)
	{
		// no changes
		if (mSettings.fullscreen == fullscreen)
			return;

		mSettings.fullscreen = fullscreen;

		// switch to fullscreen
		if (mSettings.fullscreen == true)
		{
			// store the window rect so they can be used to get back to windowed mode (also sets x,y that's who this func)
			::GetWindowRect(mWindow.get(), &mWindowed_rect);

			// change to borderless window (explicitly instead of 0)
			UINT windowStyle = WS_OVERLAPPEDWINDOW & ~(WS_CAPTION | WS_SYSMENU | WS_THICKFRAME | WS_MINIMIZEBOX | WS_MAXIMIZEBOX);

			// change the window style attribute of the window with the given style above
			::SetWindowLongPtrW(mWindow.get(), GWL_STYLE, windowStyle);

			// query the name of the nearest display device for the window and set full screen to the dominant screen. not both or any other weird way
			// get info
			HMONITOR hMonitor = ::MonitorFromWindow(mWindow.get(), MONITOR_DEFAULTTONEAREST);
			MONITORINFOEX monitorinfo = {};
			monitorinfo.cbSize = sizeof(MONITORINFOEX);
			::GetMonitorInfo(hMonitor, &monitorinfo);

			// set the position of the window (potentially calmping to one monitor if there are more than 1 monitors)
			int x, y, w, h;
			x = monitorinfo.rcMonitor.left;
			y = monitorinfo.rcMonitor.top;
			w = monitorinfo.rcMonitor.right - monitorinfo.rcMonitor.left;
			h = monitorinfo.rcMonitor.bottom - monitorinfo.rcMonitor.top;
			::SetWindowPos(mWindow.get(), HWND_TOP, x, y, w, h, SWP_FRAMECHANGED | SWP_NOACTIVATE);

			::ShowWindow(mWindow.get(), SW_MAXIMIZE);
		}
		// restore windowed size
		else
		{
			// turn back on all the decor
			::SetWindowLongPtrW(mWindow.get(), GWL_STYLE, WS_OVERLAPPEDWINDOW);
			// set the pos of the window to old pos
			int x, y, w, h;
			x = mWindowed_rect.left;
			y = mWindowed_rect.top;
			w = mWindowed_rect.right - mWindowed_rect.left;
			h = mWindowed_rect.bottom - mWindowed_rect.top;
			::SetWindowPos(mWindow.get(), HWND_NOTOPMOST, x, y, w, h, SWP_FRAMECHANGED | SWP_NOACTIVATE);

			::ShowWindow(mWindow.get(), SW_NORMAL);
		}
	}

private:
	// order matters
	MSWindow           mWindow;
	DXDevice2          mDevice;

	std::unique_ptr<DX12CommandQueue>   mCommand_queue;
	
	DXSwapChain4       mSwap_chain;
	DXDescriptorHeap   mDescriptor_heap;
	DXResource         mBack_buffers[NUMBER_OF_BUFFERS];

	DXCommandList2      mCommand_list;

	std::size_t mCurrent_back_buffer_index = 0;
	std::size_t mDescriptor_size = 0;

	RECT mWindowed_rect{};

	GFXSettings mSettings;

	// guards from window proc running GPU side event resolutions before they're created and also acts as a 'is_open'
	bool is_init = false; 
};

static LRESULT CALLBACK reserved_wnd_proc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	DX12Window* pThis = nullptr;

	if (uMsg == WM_NCCREATE)
	{
		CREATESTRUCT* pCreate = (CREATESTRUCT*)lParam;
		pThis = (DX12Window*)pCreate->lpCreateParams;
		SetWindowLongPtr(hwnd, GWLP_USERDATA, (LONG_PTR)pThis);

		pThis->mWindow = DX12Window::MSWindow(hwnd);
	}
	else
		pThis = (DX12Window*)GetWindowLongPtr(hwnd, GWLP_USERDATA);

	if (pThis)
		return pThis->wnd_proc(uMsg, wParam, lParam);
	else
		return DefWindowProc(hwnd, uMsg, wParam, lParam); // throw might be better, this should not happen in my usecase so far
}
