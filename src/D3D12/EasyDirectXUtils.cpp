#include "D3D12/EasyDirectXUtils.h"

#include <assert.h>

#include <string>
#include <stdexcept>

#include "D3D12/EasyDirectX.h"

Microsoft::WRL::ComPtr<ID3D12Resource> create_commited_resource(
	ID3D12Device4* device,
	const D3D12_HEAP_PROPERTIES& heap_properties,
	const D3D12_RESOURCE_DESC& resource_desc,
	D3D12_RESOURCE_STATES initial_state,
	D3D12_HEAP_FLAGS flags,
	const D3D12_CLEAR_VALUE* optimized_clear_value
) noexcept
{
	if (!device)
		return nullptr;

	Microsoft::WRL::ComPtr<ID3D12Resource> resource;

	HRESULT hr = device->CreateCommittedResource(
		&heap_properties,
		flags,
		&resource_desc,
		initial_state,
		optimized_clear_value,
		IID_PPV_ARGS(&resource)
	);

	if (FAILED(hr))
		return nullptr;

	return resource;
}

Microsoft::WRL::ComPtr<ID3D12Resource> create_placed_resource(
	ID3D12Device4* device, 
	ID3D12Heap* heap, 
	UINT64 heap_offset, 
	const D3D12_RESOURCE_DESC& resource_desc,
	D3D12_RESOURCE_STATES initial_state, 
	D3D12_HEAP_FLAGS flags,
	const D3D12_CLEAR_VALUE* optimized_clear_value
) noexcept
{
	if (!device || !heap)
		return nullptr;

	Microsoft::WRL::ComPtr<ID3D12Resource> resource;

	HRESULT hr = device->CreatePlacedResource(
		heap,
		heap_offset,
		&resource_desc,
		initial_state,
		optimized_clear_value,
		IID_PPV_ARGS(&resource)
	);

	if (FAILED(hr))
		return nullptr;

	return resource;
}

Microsoft::WRL::ComPtr<ID3D12Resource> copy_buffer_to_resource_and_get_intermediary(
	ID3D12Device4* device,
	ID3D12GraphicsCommandList2* command_list,
	ID3D12Resource* dest, 
	UINT64 dest_offset_in_bytes,
	const void* data, 
	UINT64 data_size_in_bytes
)
{
	if (!device || !command_list || !dest)
		throw std::invalid_argument{"invalid nullptr argument"};

	if (data_size_in_bytes == 0)
		return nullptr;

	if (!data)
		throw std::invalid_argument{ "data is nullptr" };

	const auto desc = dest->GetDesc();
	if (desc.Width < dest_offset_in_bytes + data_size_in_bytes)
		throw std::out_of_range{"out of range location specified"};

	auto intermediary = create_commited_resource(
		device,
		easy::heap_property_upload(),
		easy::resource_desc_buffer(data_size_in_bytes),
		D3D12_RESOURCE_STATE_GENERIC_READ
	);

	if (!intermediary)
		throw std::runtime_error{"something went wrong with creating a commited resource"};//should not happen if gets to this point but still

	void* CPU_local_pointer = nullptr;
	D3D12_RANGE read_range{ 0, 0 };

	// map CPU local pointer to the GPU, (with 0 read range)
	execute_and_test_hresult(
		intermediary->Map(0, &read_range, &CPU_local_pointer)
	);

	// the CPU side pointer and resources internal pointers are mapped together, so now memcpy CPU side mem copies to the resource internal pointer
	memcpy(CPU_local_pointer, data, data_size_in_bytes);

	// unmap the CPU local pointer
	intermediary->Unmap(0, nullptr);

	// issue command
	command_list->CopyBufferRegion(
		dest,
		dest_offset_in_bytes,
		intermediary.Get(),
		0,
		data_size_in_bytes
	);

	return intermediary;
}

Microsoft::WRL::ComPtr<IDXGIFactory4> create_debug_factory() noexcept
{
	UINT create_factory_flags = 0;
#if defined(_DEBUG)
	create_factory_flags = DXGI_CREATE_FACTORY_DEBUG;
#endif

	Microsoft::WRL::ComPtr<IDXGIFactory4> factory;
	HRESULT hr = CreateDXGIFactory2(create_factory_flags, IID_PPV_ARGS(&factory));

	if (FAILED(hr))
		return nullptr;

	return factory;
}

Microsoft::WRL::ComPtr<IDXGIAdapter4> find_adapter(IDXGIFactory4* factory, bool use_warp) noexcept
{
	if (!factory)
		return nullptr;

	if (use_warp) 
	{
		// CPU side rasterizer adapter thing provided by windows
		Microsoft::WRL::ComPtr<IDXGIAdapter4> warp_adapter;
		HRESULT hr = factory->EnumWarpAdapter(IID_PPV_ARGS(&warp_adapter));
		
		return FAILED(hr) ? nullptr : warp_adapter;
	}
	else
	{
		// if not using WARP, look for a GPU adapter with the largest memory pool, saving the LUID for the end
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

		// enumerate adapter by the best LUID
		Microsoft::WRL::ComPtr<IDXGIAdapter4> adapter;
		HRESULT hr = factory->EnumAdapterByLuid(best_luid, IID_PPV_ARGS(&adapter));

		return FAILED(hr) ? nullptr : adapter;
	}
}

bool check_feature_support(IDXGIFactory4* factory, DXGI_FEATURE feature) noexcept
{
	bool allow_tearing = false;
	Microsoft::WRL::ComPtr<IDXGIFactory5> factory5;
	if (SUCCEEDED(factory->QueryInterface(IID_PPV_ARGS(&factory5))))
		if (SUCCEEDED(factory5->CheckFeatureSupport(feature, &allow_tearing, sizeof(allow_tearing))))
			allow_tearing = true;

	return allow_tearing;
}

void throw_error_code_translation(DWORD error_code)
{
	// TODO:: add maybe a message box
	// TODO:: add maybe a stack trace
	LPVOID lpMsgBuf = nullptr;

	DWORD len = FormatMessageA(
		FORMAT_MESSAGE_ALLOCATE_BUFFER |
		FORMAT_MESSAGE_FROM_SYSTEM |
		FORMAT_MESSAGE_IGNORE_INSERTS,
		NULL,
		error_code,
		MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
		(LPSTR)&lpMsgBuf,
		0, NULL
	);

	if (len == 0 || lpMsgBuf == nullptr)
		throw std::runtime_error("unknown error (code " + std::to_string(error_code) + ")");

	std::string msg((LPSTR)lpMsgBuf);
	LocalFree(lpMsgBuf);
	throw std::runtime_error(msg);
}