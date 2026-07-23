#pragma once

#include "WIN32_CORE.h"
#include "GPU_CORE.h"
#include "Utils.h"

#include <memory>
#include <string>
#include "IWindowEventListener.h"
#include "DX12CommandQueue.h"
#include "Win32Window.h"
#include "DX12SwapChain.h"

class Application : public IWindowEventListener
{
	// the number of back buffers for the swap chain aka surfaces aka drawable frames
	static constexpr std::size_t NUMBER_OF_BUFFERS = 3;

	// color for clear screen
	static constexpr FLOAT G_CLEAR_SCREEN_COL[4] = { 0.4f,0.6f,0.9f,1.0f };

	// windows advanced rasterization protocol
	static constexpr bool G_IS_WARP = false;

	// doesnt make sense for these 
	Application(const Application&) = delete;
	Application& operator=(const Application&) = delete;
	Application(Application&&) = delete;
	Application& operator=(Application&&) = delete;

public:

	Application(HINSTANCE module, const std::wstring& window_title, uint32_t width, uint32_t height)
	{
		const wchar_t window_class_name[] = L"Graphics window";
		D3D12_COMMAND_QUEUE_DESC command_list_desc = {};
		command_list_desc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;            // command list type and command queue types must match. generally either: direct, compute or copy but there are others 
		command_list_desc.Priority = D3D12_COMMAND_QUEUE_PRIORITY_NORMAL;   // for rendering normal, for non sequential use high priority ( not sure for what currently )
		command_list_desc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;            // enable / disable GPU timeouts. keep default enabled
		command_list_desc.NodeMask = 0;                                     // for multi adapter systems

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
		DXGIFactory4 factory;
		UINT create_factory_flags = 0;  // only 2 values are valid: 0 or debug.
#if defined(_DEBUG)
		create_factory_flags = DXGI_CREATE_FACTORY_DEBUG;
#endif
		execute_test_throw(
			CreateDXGIFactory2(create_factory_flags, IID_PPV_ARGS(&factory))
		);

		// find a good adapter
		DXGIAdapter4 adapter4;
		execute_test_throw(
			find_adapter(factory.Get(), G_IS_WARP, adapter4)
		);

		// create device
		execute_test_throw(
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
			execute_test_throw(
				infoQueue->PushStorageFilter(&info_queue_deny_filter)
			);
		}
#endif

		// create command queue
		mCommand_queue = std::make_unique<DX12CommandQueue>(mDevice.Get(), command_list_desc);

		// create window
		mWindow = std::make_unique<Win32Window>(window_title, width, height, false, this);

		// create swapchain
		mSwapChain = mWindow->create_swap_chain(factory.Get(), mDevice.Get(), mCommand_queue->get_observer().get());

		// create command list
		// mCommand_list = mCommand_queue->get_command_list();

		//	execute_test_throw(
		//		mCommand_list->Close()
		//	);

		// set is init which up until this point protecd against sytem commands
		is_init = true;

		// show window
		//mWindow->show_window();
	}

	~Application() override = default;


	void run()
	{

		while (is_init)
		{
			// call wndproc
			MSG msg = {};
			while (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE))
			{
				//TranslateMessage(&msg); // additional messages like WM_CHAR. checking wParam manually works too. but needs research
				DispatchMessage(&msg);
			}

			begin();



			end();
		}

	}

	void begin()
	{
		mCommand_list = mCommand_queue->get_command_list();
		D3D12_RESOURCE_BARRIER barrier = mSwapChain->command_from_present_to_rtv();

		// add the command to the list
		mCommand_list->ResourceBarrier(1, &barrier);
		// index into the correct descriptor in the descriptor heap
		D3D12_CPU_DESCRIPTOR_HANDLE index = mSwapChain->command_backbuffer_handle();
		mCommand_list->ClearRenderTargetView(index, clear_color, 0, nullptr);
	}



	void end()
	{
		D3D12_RESOURCE_BARRIER barrier = mSwapChain->command_from_rtv_to_present();

		// add the command to the list
		mCommand_list->ResourceBarrier(1, &barrier);

		// signal that the recording into the list is finished
		mCommand_list->Close();

		// execute list(s)
		UINT64 signal_val = mCommand_queue->execute(mCommand_list);

		// present to display via swap chain
		mSwapChain->present(signal_val);

		// swap chain will change its internal back buffer index after calling present
		// the system is about to reuse a buffer. retrieve the new index of the buffer and stall the CPU in case the GPU is not yet done with it
		// this is a no-op generally but important safety in rare cases
		mCommand_queue->wait(mSwapChain->get_current_buffer_signal_value());
	}

	LRESULT on_message(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) override
	{
		// is_init is an important protection layer making sure no application events are processed before construction is done ( async bs )
		if (is_init) {

			mWindow->process_window_message(uMsg, wParam, lParam);

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
					if (mSwapChain->get_width() != mWindow->get_width() || mSwapChain->get_height() != mWindow->get_height())
					{
						mCommand_queue->flush();
						mSwapChain->resize_back_buffers(mWindow->get_width(), mWindow->get_height());
					}
				}
				break;

				// full screen toggle must be a special case becasue on messing with the window, something deep in win32 gets messed up.
				// currently i think the best is to approach it as a special case, hence add the event into the main event handle and not on physical key down / up
				// default window proc will play a system notification sound
			case WM_SYSKEYDOWN:
			case WM_KEYDOWN:

				//	if (wParam == VK_F11)
				//	{
				//		if (mWindow->is_fullscreen())
				//			mWindow->set_to_fullscreen();
				//		else 
				//			mWindow->set_to_windowed();
				//	}

				if (wParam == 'R')
				{
					clear_color[0] = 1.0f; clear_color[1] = 0; clear_color[2] = 0; clear_color[3] = 0;
				}

				if (wParam == 'G')
				{
					clear_color[0] = 0; clear_color[1] = 1; clear_color[2] = 0; clear_color[3] = 0;
				}

				if (wParam == 'B')
				{
					clear_color[0] = 0; clear_color[1] = 0; clear_color[2] = 1; clear_color[3] = 0;
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
		return DefWindowProcW(hwnd, uMsg, wParam, lParam);
	}
private:
	// order matters
	D3D12Device2          mDevice;

	std::unique_ptr<Win32Window>        mWindow;
	std::unique_ptr<DX12SwapChain>      mSwapChain;
	std::unique_ptr<DX12CommandQueue>   mCommand_queue;

	D3D12GraphicsCommandList2           mCommand_list;

	FLOAT clear_color[4] = { 1.0f, 0.0f, 0.0f, 1.0f };


	// guards from window proc running GPU side event resolutions before they're created and also acts as a 'is_open'
	bool is_init = false;
};
