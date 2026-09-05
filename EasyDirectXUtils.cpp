#include "EasyDirectXUtils.h"

Microsoft::WRL::ComPtr<ID3D12Resource> create_commited_resource(
	ViewPtr<ID3D12Device4> device,
	const D3D12_HEAP_PROPERTIES& heap_properties,
	const D3D12_RESOURCE_DESC& resource_desc,
	D3D12_RESOURCE_STATES initial_state,
	D3D12_HEAP_FLAGS flags,
	const D3D12_CLEAR_VALUE* optimized_clear_value
)
{
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