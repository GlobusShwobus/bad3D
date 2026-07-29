#pragma once

#include "WIN32_CORE.h"
#include "GPU_CORE.h"
#include "Utils.h"

#include <memory>
#include <string>
#include "IWindowEventListener.h"
#include "DX12CommandQueue.h"
#include "DX12RenderWindow.h"

#include "Stopwatch.h"

#include "IGame.h"
#include "Demo1.h"
#include "Demo2.h"


class Application : public IWindowEventListener
{
	// the number of back buffers for the swap chain aka surfaces aka drawable frames
	static constexpr unsigned int number_of_back_buffers = 3;

	// windows advanced rasterization protocol
	static constexpr bool using_WARP_adapter = false;

	// doesnt make sense for these 
	Application(const Application&) = delete;
	Application& operator=(const Application&) = delete;
	Application(Application&&) = delete;
	Application& operator=(Application&&) = delete;

public:

	Application() = default;

	~Application() override = default;

	void initialise_directX12()
	{
		if (directX_runtime)
			throw std::runtime_error{"attempting to reinitalise directX runtime"};

		D3D12_COMMAND_QUEUE_DESC command_queue_desc = {};
		command_queue_desc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;            // command list type and command queue types must match. generally either: direct, compute or copy but there are others 
		command_queue_desc.Priority = D3D12_COMMAND_QUEUE_PRIORITY_NORMAL;   // for rendering normal, for non sequential use high priority ( not sure for what currently )
		command_queue_desc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;            // enable / disable GPU timeouts. keep default enabled
		command_queue_desc.NodeMask = 0;                                     // for multi adapter systems

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
		UINT create_factory_flags = 0;  // only 2 values are valid: 0 or debug.
#if defined(_DEBUG)
		create_factory_flags = DXGI_CREATE_FACTORY_DEBUG;
#endif
		execute_test_throw(
			CreateDXGIFactory2(create_factory_flags, IID_PPV_ARGS(&mFactory))
		);

		// find a good adapter
		DXGIAdapter4 adapter4;
		execute_test_throw(
			find_adapter(mFactory.Get(), using_WARP_adapter, adapter4)
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
		mCommand_queue = std::make_unique<DX12CommandQueue>(command_queue_desc, mDevice.Get());

		// set all trackers to 0 initally
		mSignalTracker[0] = 0ull;
		mSignalTracker[1] = 0ull;
		mSignalTracker[2] = 0ull;

		// set flag to true
		directX_runtime = true;
	}

	void bind_game(std::unique_ptr<IGame> game)
	{
		if (!directX_runtime)
			throw std::runtime_error{ "directX runtime uninitalised" };

		if (mGame)
			mGame->unload_content();

		if (!game)
		{
			mGame.reset();
			game_bound = false;
			mDXWindow.reset();
			return;
		}
		// assign game
		mGame = std::move(game);

		if (!mDXWindow)
		{
			// first bind: actually construct window + swap chain
			mDXWindow = std::make_unique<DX12RenderWindow>(mGame->make_create_window_desc(), mFactory.Get(), mDevice.Get(), mCommand_queue.get(), number_of_back_buffers);
			mDXWindow->bind_listener(this);
		}
		else
		{
			// other binds: reconfigure existing window, let WM_SIZE drive the resize
			mCommand_queue->flush();   // make sure GPU is done with old demo's in-flight frames first
			mDXWindow->reconfigure(mGame->make_create_window_desc());
		}

		mGame->load_content();
		// set is init which up until this point protecd against sytem commands
		game_bound = true;
	}

	void run()
	{
		static Stopwatch timer;

		while (game_bound)
		{
			// call wndproc
			MSG msg = {};
			while (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE))
			{
				//TranslateMessage(&msg); // additional messages like WM_CHAR. checking wParam manually works too. but needs research
				DispatchMessage(&msg);
			}

			mGame->on_update( timer.dt_float() );

			begin();



			end();
		}

	}

	void begin()
	{
		// reset command list
		mCurrentCommandList = mCommand_queue->get_command_list();

		// set the current back buffer to go from PRESENT to RTV
		auto buffer = mDXWindow->get_buffer();
		D3D12_RESOURCE_BARRIER barrier = {};
		barrier.Type                   = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
		barrier.Flags                  = D3D12_RESOURCE_BARRIER_FLAG_NONE;
		barrier.Transition.pResource   = buffer.get();
		barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_PRESENT;         // this knowledge is implied and for back buffers it's fine because they're only ever in one or the other state. for other resources that get more complex logic, state tracking becomes important
		barrier.Transition.StateAfter  = D3D12_RESOURCE_STATE_RENDER_TARGET;
		barrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
		mCurrentCommandList->ResourceBarrier(1, &barrier);

		// NOTE: maybe should be game logic here instead
		// clear screen
		mCurrentCommandList->ClearRenderTargetView(mDXWindow->get_buffer_desc(), clear_color, 0 ,nullptr);
	}

	void end()
	{
		// set the current back buffer to go from RTV to PRESENT
		auto buffer = mDXWindow->get_buffer();
		D3D12_RESOURCE_BARRIER barrier = {};
		barrier.Type                   = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
		barrier.Flags                  = D3D12_RESOURCE_BARRIER_FLAG_NONE;
		barrier.Transition.pResource   = buffer.get();
		barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_RENDER_TARGET;         // this knowledge is implied and for back buffers it's fine because they're only ever in one or the other state. for other resources that get more complex logic, state tracking becomes important
		barrier.Transition.StateAfter  = D3D12_RESOURCE_STATE_PRESENT;
		barrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
		mCurrentCommandList->ResourceBarrier(1, &barrier);

		// the final command to the list has been recorded. close the list.
		mCurrentCommandList->Close();


		// ask for the current back buffer index, 
		// then execute and get signal value,
		// then cache signal value,
		// then present which updates current index and reseats back buffer which means i must potentially stall the CPU if the whatever new back buffer is not actually free yet
		// then check signal value before resuing some unknown buffer
		const UINT64 current_index = mDXWindow->get_buffer_index();
		const UINT64 signal_val = mCommand_queue->execute(mCurrentCommandList);
		mSignalTracker[current_index] = signal_val;

		mDXWindow->present();

		const UINT64 some_new_buffer_index = mDXWindow->get_buffer_index();

		mCommand_queue->wait( mSignalTracker[some_new_buffer_index] );
	}

	LRESULT on_message(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) override
	{
		// is_init is an important protection layer making sure no application events are processed before construction is done ( async bs )
		if (game_bound) {

			switch (uMsg)
			{
			case WM_DESTROY:
				PostQuitMessage(0);
				game_bound = false;
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
					mDXWindow->on_resize();
				}
				break;

				// full screen toggle must be a special case becasue on messing with the window, something deep in win32 gets messed up.
				// currently i think the best is to approach it as a special case, hence add the event into the main event handle and not on physical key down / up
				// default window proc will play a system notification sound
			case WM_SYSKEYDOWN:
			case WM_KEYDOWN:

				if (wParam == VK_F11)
				{
					mDXWindow->on_fullscreen_transition();
				}

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

				if (wParam == 'N')
				{
					bind_game(std::make_unique<Demo1>());
				}

				if(wParam == 'M')
				{
					bind_game(std::make_unique<Demo2>());
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

	void reset_signal_tracker()
	{
		const UINT current = mDXWindow->get_buffer_index();
		for (UINT i = 0; i < number_of_back_buffers; i++)
			mSignalTracker[i] = mSignalTracker[current];
	}
private:
	// order matters
	DXGIFactory4          mFactory;
	D3D12Device2          mDevice;

	std::unique_ptr<DX12RenderWindow>   mDXWindow;
	std::unique_ptr<DX12CommandQueue>   mCommand_queue;

	D3D12GraphicsCommandList2           mCurrentCommandList;
	D3D12Resource                       mCurrentBackBuffer;
	UINT64                              mSignalTracker[number_of_back_buffers];

	std::unique_ptr<IGame> mGame = nullptr;

	FLOAT clear_color[4] = { 1.0f, 0.0f, 0.0f, 1.0f };

	bool directX_runtime = false;
	bool game_bound = false;
};
