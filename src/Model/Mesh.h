#pragma once

#include <d3d12.h>
#include <wrl/client.h>

#include "Tools/ViewPtr.h"
#include "D3D12/Resource.h"
#include "D3D12/BufferViews.h"

class Mesh
{
public:

	Mesh() = default;
	Mesh(const Resource& vertex_buffer, UINT vertex_stride, const Resource& index_buffer);
	Mesh(
		ViewPtr<Resource> vertex_buffer,
		UINT64 vertex_byte_position,
		UINT vertex_byte_count,
		UINT vertex_stride,
		ViewPtr<Resource> index_buffer,
		UINT64 index_byte_position,
		UINT index_byte_count,
		DXGI_FORMAT index_format
	);

	const D3D12_VERTEX_BUFFER_VIEW& get_vertex_view() const noexcept { return mVertexView.view(); }
	const D3D12_INDEX_BUFFER_VIEW& get_index_view() const noexcept { return mIndexView.view(); }

	UINT get_index_count() const noexcept {
		return mIndexView.element_count();
	}

	//VertexBuffer* vertex_buffer() noexcept { return mVertexBuffer.get(); }
	//IndexBuffer* index_buffer() noexcept { return mIndexBuffer.get(); }

private:

	ViewPtr<Resource> mVertexBuffer = nullptr;
	VertexBufferView mVertexView;

	ViewPtr<Resource> mIndexBuffer = nullptr;
	IndexBufferView mIndexView;
};