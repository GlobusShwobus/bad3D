#include "EasyDirectXUtils.h"
#include "EasyDirectX.h"
#include <assert.h>

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

Microsoft::WRL::ComPtr<ID3D12Resource> copy_buffer_to_resource_and_get_intermediary(ID3D12Device4* device, ID3D12GraphicsCommandList2* command_list, ID3D12Resource* dest, const void* data, SIZE_T element_count, SIZE_T type_size)
{
	assert(device && "nullptr");
	assert(command_list && "nullptr");
	assert(dest && "nullptr");
	assert(type_size > 0);

	if (data == nullptr || element_count == 0)
		return nullptr;

	UINT64 byte_count = element_count * type_size;
	auto intermediary = create_commited_resource(
		device,
		HEAP_PROPERTY::upload(),
		RESOURCE_DESC::buffer(byte_count),
		D3D12_RESOURCE_STATE_GENERIC_READ
	);

	void* CPU_local_pointer = nullptr;
	D3D12_RANGE read_range{ 0, 0 };

	// map CPU local pointer to the GPU, (with 0 read range)
	intermediary->Map(0, &read_range, &CPU_local_pointer);

	// the CPU side pointer and resources internal pointers are mapped together, so now memcpy CPU side mem copies to the resource internal pointer
	memcpy(CPU_local_pointer, data, byte_count);

	// unmap the CPU local pointer
	intermediary->Unmap(0, nullptr);

	// issue command
	command_list->CopyBufferRegion(
		dest,
		0,
		intermediary.Get(),
		0,
		byte_count
	);

	return intermediary;
}
