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
)
{
	assert(device && "nullptr");

	Microsoft::WRL::ComPtr<ID3D12Resource> resource = nullptr;

	device->CreateCommittedResource(
		&heap_properties,
		flags,
		&resource_desc,
		initial_state,
		optimized_clear_value,
		IID_PPV_ARGS(&resource)
	);

	return resource;
}

Microsoft::WRL::ComPtr<ID3D12Resource> create_placed_resource(ID3D12Device4* device, ID3D12Heap* heap, UINT64 heap_offset, const D3D12_RESOURCE_DESC& resource_desc, D3D12_RESOURCE_STATES initial_state, D3D12_HEAP_FLAGS flags, const D3D12_CLEAR_VALUE* optimized_clear_value)
{
	assert(device && "nullptr");
	assert(heap && "nullptr");

	Microsoft::WRL::ComPtr<ID3D12Resource> resource = nullptr;

	device->CreatePlacedResource(
		heap,
		heap_offset,
		&resource_desc,
		initial_state,
		optimized_clear_value,
		IID_PPV_ARGS(&resource)
	);

	return resource;
}

Microsoft::WRL::ComPtr<ID3D12Resource> copy_buffer_to_resource_and_get_intermediary(ID3D12Device4* device, ID3D12GraphicsCommandList2* command_list, ID3D12Resource* dest, UINT64 dest_offset, const void* data, UINT64 byte_size)
{
	assert(device && "nullptr");
	assert(command_list && "nullptr");
	assert(dest && "nullptr");

	if (data == nullptr || byte_size == 0)
		return nullptr;

	auto intermediary = create_commited_resource(
		device,
		easy::heap_property_upload(),
		easy::resource_desc_buffer(byte_size),
		D3D12_RESOURCE_STATE_GENERIC_READ
	);

	void* CPU_local_pointer = nullptr;
	D3D12_RANGE read_range{ 0, 0 };

	// map CPU local pointer to the GPU, (with 0 read range)
	intermediary->Map(0, &read_range, &CPU_local_pointer);

	// the CPU side pointer and resources internal pointers are mapped together, so now memcpy CPU side mem copies to the resource internal pointer
	memcpy(CPU_local_pointer, data, byte_size);

	// unmap the CPU local pointer
	intermediary->Unmap(0, nullptr);

	// issue command
	command_list->CopyBufferRegion(
		dest,
		dest_offset,
		intermediary.Get(),
		0,
		byte_size
	);

	return intermediary;
}

Microsoft::WRL::ComPtr<IDXGIAdapter4> find_adapter(IDXGIFactory4* factory, bool use_warp)
{
	assert(factory && "factory nullptr");

	HRESULT hr = E_FAIL;
	Microsoft::WRL::ComPtr<IDXGIAdapter4> adapter4;
	if (use_warp) // since WARP is a specific adapter, just get it directly. EnumWarpAdapter takes type void as param, so query interface works as expected.
	{
		hr = factory->EnumWarpAdapter(IID_PPV_ARGS(&adapter4));
	}
	else         // if not using WARP, need to look for an adapter
	{
		// first, if looking for adapter manually, it is not possible to enumerate with Adapter4 since EnumAdapters and EnumAdapters1 take specific types.
		// secondly, need to find adapter with a good amount of memory...
		LUID best_luid = {};
		Microsoft::WRL::ComPtr<IDXGIAdapter1> adapter1;
		SIZE_T largest_memory_pool = 0;

		for (UINT adapterIndex = 0; ; ++adapterIndex)
		{
			// if reached end of the line
			if (factory->EnumAdapters1(adapterIndex, &adapter1) == DXGI_ERROR_NOT_FOUND)
				break;

			DXGI_ADAPTER_DESC1 desc1;
			adapter1->GetDesc1(&desc1);

			// ignore software adapters
			if ((desc1.Flags & DXGI_ADAPTER_FLAG_SOFTWARE) == 0)
			{
				// call create device to check if it succeeds but don't instantiate the type, by passing nullptr to output
				if (SUCCEEDED(D3D12CreateDevice(adapter1.Get(), D3D_FEATURE_LEVEL_11_0, __uuidof(ID3D12Device), nullptr)))
				{
					// if create device runs successfully then store the dedicated mem size and LUID and later actually enumerate the adapter by LUID
					if (desc1.DedicatedVideoMemory > largest_memory_pool)
					{
						largest_memory_pool = desc1.DedicatedVideoMemory;
						best_luid = desc1.AdapterLuid;
					}
				}
			}

			adapter1.Reset();
		}

		// enumerate adapter by the best LUID
		hr = factory->EnumAdapterByLuid(best_luid, IID_PPV_ARGS(&adapter4));
	}

	return adapter4;
}

bool check_feature_support(IDXGIFactory4* factory, DXGI_FEATURE feature)
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

void execute_and_test_hresult(HRESULT hr)
{
	if (FAILED(hr))
		throw_error_code_translation(static_cast<DWORD>(hr));
}

void execute_and_test_BOOL(BOOL b)
{
	if (b == 0)
		throw_error_code_translation(GetLastError());
}