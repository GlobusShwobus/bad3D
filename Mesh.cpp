#include "Mesh.h"

Microsoft::WRL::ComPtr<ID3D12Resource> Mesh::load_index_buffer(ID3D12Device4* device, ID3D12GraphicsCommandList2* cl, const std::vector<WORD>& buffer)
{
	assert(device && "nullptr");
	assert(cl && "nullptr");

	const SIZE_T type_size = sizeof(WORD);
	const SIZE_T element_count = buffer.size();

	auto intermediary = mIndexBuffer.load(
		device,
		cl,
		buffer.data(),
		element_count,
		type_size
	);

	mIndexBufferView = RESOURCE_VIEW::index(
		mIndexBuffer.get()->GetGPUVirtualAddress(),
		element_count * type_size,
		DXGI_FORMAT_R16_UINT
	);

	return intermediary;
}
