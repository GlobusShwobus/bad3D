#pragma once

#include <d3d12.h>
#include <wrl/client.h>

#include "Tools/ViewPtr.h"
#include "Model/ModelBuffer.h"

class Mesh
{
public:

	Mesh() = default;
	Mesh(ViewPtr<VertexBuffer> vertex_buffer, ViewPtr<IndexBuffer> index_buffer);
	Mesh(
		ViewPtr<VertexBuffer> vertex_buffer,
		UINT64 vertex_byte_position,
		UINT vertex_byte_count,
		ViewPtr<IndexBuffer> index_buffer,
		UINT64 index_byte_position,
		UINT index_byte_count
	);

	const D3D12_VERTEX_BUFFER_VIEW& get_vertex_view() const noexcept { return mVertexView; }
	const D3D12_INDEX_BUFFER_VIEW& get_index_view() const noexcept { return mIndexView; }
	UINT get_index_count() const noexcept {
		return mIndexView.SizeInBytes / IndexBuffer::index_stride_from_format(mIndexView.Format);
	}

	VertexBuffer* vertex_buffer() noexcept { return mVertexBuffer.get(); }
	IndexBuffer* index_buffer() noexcept { return mIndexBuffer.get(); }

private:

	ViewPtr<VertexBuffer> mVertexBuffer = nullptr;
	D3D12_VERTEX_BUFFER_VIEW mVertexView = {};

	ViewPtr<IndexBuffer> mIndexBuffer = nullptr;
	D3D12_INDEX_BUFFER_VIEW mIndexView = {};
};