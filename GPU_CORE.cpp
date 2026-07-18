#include "GPU_CORE.h"

HRESULT find_adapter(ObserverPtr<IDXGIFactory4> factory, bool use_warp, DXAdapter4& adapter_out)
{
	HRESULT hr = E_FAIL;
	if (use_warp) // since WARP is a specific adapter, just get it directly. EnumWarpAdapter takes type void as param, so query interface works as expected.
	{
		hr = factory->EnumWarpAdapter(IID_PPV_ARGS(&adapter_out));
	}
	else         // if not using WARP, need to look for an adapter
	{
		// first, if looking for adapter manually, it is not possible to enumerate with Adapter4 since EnumAdapters and EnumAdapters1 take specific types.
		// secondly, need to find adapter with a good amount of memory...
		LUID best_luid = {};
		DXAdapter1 adapter_t1 = nullptr;
		SIZE_T largest_memory_pool = 0;

		for (UINT adapterIndex = 0; ; ++adapterIndex)
		{
			// if reached end of the line
			if (factory->EnumAdapters1(adapterIndex, &adapter_t1) == DXGI_ERROR_NOT_FOUND)
				break;

			DXGI_ADAPTER_DESC1 desc1;
			adapter_t1->GetDesc1(&desc1);

			// ignore software adapters
			if ((desc1.Flags & DXGI_ADAPTER_FLAG_SOFTWARE) == 0)
			{
				// call create device to check if it succeeds but don't instantiate the type, by passing nullptr to output
				if (SUCCEEDED(D3D12CreateDevice(adapter_t1.Get(), D3D_FEATURE_LEVEL_11_0, __uuidof(ID3D12Device), nullptr)))
				{
					// if create device runs successfully then store the dedicated mem size and LUID and later actually enumerate the adapter by LUID
					if (desc1.DedicatedVideoMemory > largest_memory_pool)
					{
						largest_memory_pool = desc1.DedicatedVideoMemory;
						best_luid = desc1.AdapterLuid;
					}
				}
			}

			adapter_t1.Reset();
		}

		// enumerate adapter by the best LUID
		hr = factory->EnumAdapterByLuid(best_luid, IID_PPV_ARGS(&adapter_out));
	}

	return hr;
}

// in release mode this function always returns S_OK
HRESULT enable_GPU_debug_layer()
{
	HRESULT hr = S_OK;
#if defined(_DEBUG)
	// Always enable the debug layer before doing anything DX12 related
	// so all possible errors generated while creating DX12 objects
	// are caught by the debug layer.
	Microsoft::WRL::ComPtr<ID3D12Debug> debugInterface;
	hr = D3D12GetDebugInterface(IID_PPV_ARGS(&debugInterface));

	if (SUCCEEDED(hr))
		debugInterface->EnableDebugLayer();
#endif
	return hr;
}

bool check_is_tearing_supported(ObserverPtr<IDXGIFactory4> factory)
{
	BOOL allow_tearing = FALSE;
	// Rather than create the 1.5 factory interface directly, we create the 1.4
	// interface and query for the 1.5 interface. This will enable the graphics
	// debugging tools which might not support the 1.5 factory interface.
	DXFactory5 factory5;

	if (SUCCEEDED(factory->QueryInterface(IID_PPV_ARGS(&factory5))))
	{
		if (FAILED(factory5->CheckFeatureSupport(DXGI_FEATURE_PRESENT_ALLOW_TEARING, &allow_tearing, sizeof(allow_tearing))))
		{
			allow_tearing = FALSE;
		}
	}
	return allow_tearing == TRUE;
}