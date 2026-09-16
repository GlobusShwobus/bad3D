#pragma once

#include <d3d12.h>
#include <dxgi1_6.h>
#include <wrl/client.h>

Microsoft::WRL::ComPtr<ID3D12Resource> create_commited_resource(
	ID3D12Device4* device,
	const D3D12_HEAP_PROPERTIES& heap_properties,
	const D3D12_RESOURCE_DESC& resource_desc,
	D3D12_RESOURCE_STATES initial_state,
	D3D12_HEAP_FLAGS flags = D3D12_HEAP_FLAG_NONE,
	const D3D12_CLEAR_VALUE* optimized_clear_value = nullptr
);

Microsoft::WRL::ComPtr<ID3D12Resource> create_placed_resource(
	ID3D12Device4* device,
	ID3D12Heap* heap,
	UINT64 heap_offset,
	const D3D12_RESOURCE_DESC& resource_desc,
	D3D12_RESOURCE_STATES initial_state,
	D3D12_HEAP_FLAGS flags = D3D12_HEAP_FLAG_NONE,
	const D3D12_CLEAR_VALUE* optimized_clear_value = nullptr
);


Microsoft::WRL::ComPtr<ID3D12Resource> copy_buffer_to_resource_and_get_intermediary(ID3D12Device4* device, ID3D12GraphicsCommandList2* command_list, ID3D12Resource* dest, const void* data, SIZE_T byte_size);

Microsoft::WRL::ComPtr<IDXGIAdapter4> find_adapter(IDXGIFactory4* factory, bool use_warp);