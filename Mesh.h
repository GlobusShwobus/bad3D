#pragma once

#include <vector>
#include <d3d12.h>
#include <wrl/client.h>
#include "Resource.h"
#include <assert.h>

#include "EasyDirectX.h"
#include "EasyDirectXUtils.h"
class Mesh
{
public:

	template <typename vertex>
	Microsoft::WRL::ComPtr<ID3D12Resource> load_vertex_buffer(ID3D12Device4* device, ID3D12GraphicsCommandList2* cl, const std::vector<vertex>& buffer)
	{
		assert(device && "nullptr");
		assert(cl && "nullptr");

		const SIZE_T type_size = sizeof(vertex);
		const SIZE_T element_count = buffer.size();

		auto intermediary = mVertexBuffer.load(
			device,
			cl,
			buffer.data(),
			element_count,
			type_size
		);

		mVertexBufferView = RESOURCE_VIEW::vertex(
			mVertexBuffer.get()->GetGPUVirtualAddress(),
			element_count * type_size,
			type_size
		);

		return intermediary;
	}

	Microsoft::WRL::ComPtr<ID3D12Resource> load_index_buffer(ID3D12Device4* device, ID3D12GraphicsCommandList2* cl, const std::vector<WORD>& buffer);


	const D3D12_VERTEX_BUFFER_VIEW& vertex_view() const noexcept { return mVertexBufferView; }
	const D3D12_INDEX_BUFFER_VIEW& index_view() const noexcept { return mIndexBufferView; }
	
	const SIZE_T index_count() const noexcept { return mIndexBuffer.count(); }

private:

	CommittedResource mVertexBuffer;
	D3D12_VERTEX_BUFFER_VIEW mVertexBufferView = {};

	CommittedResource mIndexBuffer;
	D3D12_INDEX_BUFFER_VIEW mIndexBufferView = {};
};