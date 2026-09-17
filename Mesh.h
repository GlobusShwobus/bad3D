#pragma once

#include <d3d12.h>
#include <wrl/client.h>

#include "ViewPtr.h"
#include "Resource.h"

class Mesh
{
public:

	Mesh() = default;
	Mesh(ViewPtr<VertexBuffer> vertex_buffer, ViewPtr<IndexBuffer> index_buffer)
		:mVertexBuffer(vertex_buffer), mIndexBuffer(index_buffer)
	{
	}

	void set_vertex_view(UINT byte_position, UINT byte_count)
	{
		mVertexView = mVertexBuffer->create_subview(byte_position, byte_count);
	}

	void set_index_view(UINT byte_position, UINT byte_count, UINT element_count)
	{
		mIndexView = mIndexBuffer->create_subview(byte_position, byte_count);
		mIndexCount = element_count;
	}

	const D3D12_VERTEX_BUFFER_VIEW& get_vertex_view() const noexcept { return mVertexView; }
	const D3D12_INDEX_BUFFER_VIEW& get_index_view() const noexcept { return mIndexView; }
	UINT get_index_count() const noexcept { return mIndexCount; }

	VertexBuffer* vertex_buffer() noexcept { return mVertexBuffer.get(); }
	IndexBuffer* index_buffer() noexcept { return mIndexBuffer.get(); }

private:

	ViewPtr<VertexBuffer> mVertexBuffer;
	D3D12_VERTEX_BUFFER_VIEW mVertexView;

	ViewPtr<IndexBuffer> mIndexBuffer;
	D3D12_INDEX_BUFFER_VIEW mIndexView;
	UINT mIndexCount;
};