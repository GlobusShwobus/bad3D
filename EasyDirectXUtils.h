#pragma once

#include "EasyDirectX.h"
#include <wrl/client.h>
#include "ViewPtr.h"
#include <vector>

Microsoft::WRL::ComPtr<ID3D12Resource> create_commited_resource(
	ViewPtr<ID3D12Device4> device,
	const D3D12_HEAP_PROPERTIES& heap_properties,
	const D3D12_RESOURCE_DESC& resource_desc,
	D3D12_RESOURCE_STATES initial_state,
	D3D12_HEAP_FLAGS flags = D3D12_HEAP_FLAG_NONE,
	const D3D12_CLEAR_VALUE* optimized_clear_value = nullptr
);

template<typename T>
Microsoft::WRL::ComPtr<ID3D12Resource> copy_buffer_to_resource_and_get_intermediary(ViewPtr<ID3D12Device4> device, ViewPtr<ID3D12GraphicsCommandList2> command_list, ViewPtr<ID3D12Resource> dest, const std::vector<T>& data)
{
	if (data.empty())
		return nullptr;

	std::size_t num_bytes = data.size() * sizeof(T);

	auto intermediary = create_commited_resource(
		device,
		HEAP_PROPERTY::upload(),
		RESOURCE_DESC::buffer(num_bytes),
		D3D12_RESOURCE_STATE_GENERIC_READ
	);

	// map CPU local pointer to the GPU, (with 0 read range)
	void* CPU_local_pointer = nullptr;
	D3D12_RANGE read_range{ 0, 0 };
	intermediary->Map(0, &read_range, &CPU_local_pointer);

	// the CPU side pointer and resources internal pointers are mapped together, so now memcpy CPU side mem copies to the resource internal pointer
	memcpy(CPU_local_pointer, data.data(), num_bytes);

	// unmap the CPU local pointer
	intermediary->Unmap(0, nullptr); 

	// issue command
	command_list->CopyBufferRegion(
		dest.get(),
		0,
		intermediary.Get(),
		0,
		num_bytes
	);

	return intermediary;
}