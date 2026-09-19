#include "Model/Mesh.h"

#include <assert.h>

Mesh::Mesh(ViewPtr<VertexBuffer> vertex_buffer, ViewPtr<IndexBuffer> index_buffer)
	:Mesh(vertex_buffer, 0, vertex_buffer->size_in_bytes(), index_buffer, 0, index_buffer->size_in_bytes())
{
}

Mesh::Mesh(
	ViewPtr<VertexBuffer> vertex_buffer,
	UINT64 vertex_byte_position,
	UINT vertex_byte_count,
	ViewPtr<IndexBuffer> index_buffer,
	UINT64 index_byte_position,
	UINT index_byte_count
)
	:mVertexBuffer(vertex_buffer), mIndexBuffer(index_buffer)
{
	assert(mVertexBuffer && mVertexBuffer.get() && "nullptr");
	assert(mIndexBuffer && mIndexBuffer.get() && "nullptr");

	mVertexView = mVertexBuffer->create_subview(vertex_byte_position, vertex_byte_count);
	mIndexView = mIndexBuffer->create_subview(index_byte_position, index_byte_count);
}