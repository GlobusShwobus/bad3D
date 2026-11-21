#include "App/GraphicsDevice.h"


#include <memory>
#include <stdexcept>
#include <assert.h>

#include <dxgi1_6.h>
#include <wrl/client.h>

#include "App/CommandQueue.h"
#include "D3D12/EasyDirectXUtils.h"

namespace
{
	class GraphicsDevice
	{
	public:
		GraphicsDevice() = default;
		~GraphicsDevice() {
			flush_all(); 
		}
		GraphicsDevice(const GraphicsDevice&) = delete;
		GraphicsDevice& operator=(const GraphicsDevice&) = delete;

		bool init(bool WARP_adapter) {
			// create DXGI factory
			UINT DXGI_factory_flags = 0;
#if defined(_DEBUG)
			DXGI_factory_flags = DXGI_CREATE_FACTORY_DEBUG;
#endif

			Microsoft::WRL::ComPtr<IDXGIFactory4> factory;
			execute_and_test_hresult(
				::CreateDXGIFactory2(DXGI_factory_flags, IID_PPV_ARGS(&factory))
			);

			// create adapter
			Microsoft::WRL::ComPtr<IDXGIAdapter4> adapter;

			if (WARP_adapter) {
				execute_and_test_hresult(
					factory->EnumWarpAdapter(IID_PPV_ARGS(&adapter))
				);
			}
			else {
				LUID best_luid = {};
				SIZE_T largest_memory_pool = 0;

				for (UINT adapterIndex = 0; ; ++adapterIndex)
				{
					// if reached the end
					Microsoft::WRL::ComPtr<IDXGIAdapter1> adapter;
					if (factory->EnumAdapters1(adapterIndex, &adapter) == DXGI_ERROR_NOT_FOUND)
						break;

					DXGI_ADAPTER_DESC1 desc1;
					adapter->GetDesc1(&desc1);

					// ignore software adapters
					if ((desc1.Flags & DXGI_ADAPTER_FLAG_SOFTWARE) != FALSE)
						continue;

					// check if the call to create device succeeds without instantiating the obj
					if (SUCCEEDED(D3D12CreateDevice(adapter.Get(), D3D_FEATURE_LEVEL_11_0, __uuidof(ID3D12Device), nullptr)))
					{
						// if good, store for later
						if (desc1.DedicatedVideoMemory > largest_memory_pool)
						{
							largest_memory_pool = desc1.DedicatedVideoMemory;
							best_luid = desc1.AdapterLuid;
						}
					}
				}
				execute_and_test_hresult(
					factory->EnumAdapterByLuid(best_luid, IID_PPV_ARGS(&adapter))
				);
			}

			// create device
			execute_and_test_hresult(
				D3D12CreateDevice(adapter.Get(), D3D_FEATURE_LEVEL_11_0, IID_PPV_ARGS(&mDevice))
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

			mDirect = std::make_unique<CommandQueue>(mDevice.Get(), D3D12_COMMAND_LIST_TYPE_DIRECT);
			mCompute = std::make_unique<CommandQueue>(mDevice.Get(), D3D12_COMMAND_LIST_TYPE_COMPUTE);
			mCopy = std::make_unique<CommandQueue>(mDevice.Get(), D3D12_COMMAND_LIST_TYPE_COPY);

			if (!mDirect || !mCompute || !mCopy)
				throw std::runtime_error("failed to create command queue(s)");

			return true;
		}

		ID3D12Device4* get_device() const noexcept { return mDevice.Get(); }
		CommandQueue*  get_direct_queue()  noexcept { return mDirect.get(); }
		CommandQueue*  get_compute_queue() noexcept { return mCompute.get(); }
		CommandQueue*  get_copy_queue()    noexcept { return mCopy.get(); }

		void flush_all() {
			if (mDirect)  
				mDirect->flush();
			if (mCompute) 
				mCompute->flush();
			if (mCopy)   
				mCopy->flush();
		}

	private:

		Microsoft::WRL::ComPtr<ID3D12Device4> mDevice;
		std::unique_ptr<CommandQueue>         mDirect;
		std::unique_ptr<CommandQueue>         mCompute;
		std::unique_ptr<CommandQueue>         mCopy;
	};

	std::unique_ptr<GraphicsDevice> gDevice;
}

namespace GFX 
{
	bool device_init(bool WARP_adapter)
	{
		if (gDevice)
			throw std::runtime_error{ "device already initialized" };

		auto device = std::make_unique<GraphicsDevice>();

		if (!device->init(WARP_adapter))
			return false;

		gDevice = std::move(device);
		return true;
	}

	bool device_destroy()
	{
		if (!gDevice)
			return false;

		gDevice.reset();
		return true;
	}

	void device_flush_all()
	{
		if (!gDevice)
			return;
		gDevice->flush_all();
	}

	ViewPtr<ID3D12Device4> get_device() noexcept
	{
		assert(gDevice);
		return ViewPtr{ gDevice->get_device() };
	}

	ViewPtr<CommandQueue> get_direct_queue()  noexcept { 
		assert(gDevice);
		return ViewPtr{ gDevice->get_direct_queue() };
	}

	ViewPtr<CommandQueue> get_compute_queue() noexcept { 
		assert(gDevice);
		return ViewPtr{ gDevice->get_compute_queue() };
	}

	ViewPtr<CommandQueue> get_copy_queue()    noexcept { 
		assert(gDevice);
		return ViewPtr{ gDevice->get_copy_queue() };
	}
}
