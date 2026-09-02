#pragma once

#include <vector>
#include "badWin32.h"
#include <assert.h>

template<typename vertex>
class Mesh
{
public:
	Mesh() = default;
	Mesh(std::vector<vertex> vertecies, std::vector<WORD> indecies)
		:mVertexBuffer(std::move(vertecies)), mIndexBuffer(std::move(indecies))
	{
		assert(mVertexBuffer.size() > 2);
		assert(mIndexBuffer.size() % 3 == 0);
	}

	constexpr std::size_t vertex_count() const noexcept       { return mVertexBuffer.size(); }
	constexpr std::size_t vertex_type_size() const noexcept   { return sizeof(vertex); }
	constexpr std::size_t vertex_buffer_size() const noexcept { return mVertexBuffer.size() * sizeof(vertex); }

	constexpr std::size_t index_count() const noexcept        { return mIndexBuffer.size(); }
	constexpr std::size_t index_type_size() const noexcept    { return sizeof(WORD); }
	constexpr std::size_t index_buffer_size() const noexcept  { return mIndexBuffer.size() * sizeof(WORD); }

	std::vector<vertex> mVertexBuffer;
	std::vector<WORD> mIndexBuffer;
};