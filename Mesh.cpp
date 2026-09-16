#include "Mesh.h"

Microsoft::WRL::ComPtr<ID3D12Resource> Mesh::load_index_buffer(ID3D12Device4* device, ID3D12GraphicsCommandList2* cl, const std::vector<WORD>& buffer)
{
	assert(device && "nullptr");
	assert(cl && "nullptr");

	const UINT64 type_size = sizeof(WORD);
	const UINT64 element_count = buffer.size();
	const UINT64 byte_size = element_count * type_size;

	auto resource = Resource::create_commited(
		device,
		HEAP_PROPERTY::base(),
		RESOURCE_DESC::buffer(byte_size),
		D3D12_RESOURCE_STATE_COMMON
	);

	mIndexBuffer = std::move( IndexBuffer{std::move(resource), element_count, DXGI_FORMAT_R16_UINT } );

	auto intermediary = copy_buffer_to_resource_and_get_intermediary(
		device,
		cl,
		mIndexBuffer.get(),
		buffer.data(),
		byte_size
	);

	mIndexBufferView = RESOURCE_VIEW::index(
		mIndexBuffer.get()->GetGPUVirtualAddress(),
		byte_size,
		DXGI_FORMAT_R16_UINT
	);

	return intermediary;
}
