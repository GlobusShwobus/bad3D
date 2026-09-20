#include "App/GraphicsDevice.h"

#include <dxgi1_6.h>

#include "D3D12/EasyDirectXUtils.h"

GraphicsDevice::GraphicsDevice()
{
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
	mDirect = CommandQueue{ device_, D3D12_COMMAND_LIST_TYPE_DIRECT };
	mCompute = CommandQueue{ device_, D3D12_COMMAND_LIST_TYPE_COMPUTE };
	mCopy = CommandQueue{ device_, D3D12_COMMAND_LIST_TYPE_COPY };

	mInitialised = true;
}

void GraphicsDevice::flush_all()
{
	if (mInitialised) {
		mDirect.flush_execution();
		mCompute.flush_execution();
		mCopy.flush_execution();
	}
}