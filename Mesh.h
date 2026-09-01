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
		:mVertex(std::move(vertecies)), mIndex(std::move(indecies))
	{
		assert(mVertex.size() > 2);
		assert(mIndex.size() % 3 == 0);
	}

	constexpr std::size_t vertex_count() const noexcept { return mVertex.size(); }
	constexpr std::size_t index_count() const noexcept { return mIndex.size(); }

	constexpr vertex* vertex_data()  noexcept { return mVertex.data(); }
	constexpr WORD* index_data()  noexcept { return mIndex.data(); }

	std::vector<vertex> mVertex;
	std::vector<WORD> mIndex;
};