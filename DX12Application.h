#pragma once

#include "GPU_CORE.h"
#include "Utils.h"

#if defined(min)
#undef min
#endif

#if defined(max)
#undef max
#endif

#include <memory>
#include "IWindowEventListener.h"
#include "DX12CommandQueue.h"
#include "Win32Window.h"


class DX12Application : public IWindowEventListener
{
	// the number of back buffers for the swap chain aka surfaces aka drawable frames
	static constexpr std::size_t NUMBER_OF_BUFFERS = 3;

	// color for clear screen
	static constexpr FLOAT G_CLEAR_SCREEN_COL[4] = { 0.4f,0.6f,0.9f,1.0f };

	// windows advanced rasterization protocol
	static constexpr bool G_IS_WARP = false;

	// doesnt make sense for these 
	DX12Application(const DX12Application&) = delete;
	DX12Application& operator=(const DX12Application&) = delete;
	DX12Application(DX12Application&&) = delete;
	DX12Application& operator=(DX12Application&&) = delete;

public:

	DX12Application(HINSTANCE module, const wchar_t* title, uint32_t width, uint32_t height, DWORD window_style)
	{
		const wchar_t window_class_name[] = L"Graphics window";
		const D3D12_COMMAND_LIST_TYPE command_list_type = D3D12_COMMAND_LIST_TYPE_DIRECT;

		// command queue description

		// swap chain description


		// descriptor heap description


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


		// create window. the window is created as a temporary and actually assigned in window procedure on WM_NCCREATE


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

		// create the swap chain


		// set the the back buffer tracking index


		// create descriptor heap and descriptor size

		// cache the system descriptor size


		// create RTV buffer handles and descriptors


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
		ShowWindow(mWindow.mHwnd, SW_SHOW);
	}

	~DX12Application() override = default;

	LRESULT on_message(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) override
	{
		return 0;
	}
private:
	// order matters
	Win32Window        mWindow;
	DXDevice2          mDevice;

	std::unique_ptr<DX12CommandQueue>   mCommand_queue;

	

	DXCommandList2      mCommand_list;


	// guards from window proc running GPU side event resolutions before they're created and also acts as a 'is_open'
	bool is_init = false;
};
