#pragma once

#include <utility>
#include <vector>

#include <d3d12.h>
#include <wrl/client.h>

#include "D3D12/Resource.h"
#include "D3D12/BufferViews.h"

struct Submesh
{
	VertexBufferView vertex_view;
	IndexBufferView index_view;
};

// as things go ideally shit is not as exposed

class Mesh // mesh for textured objects????? idk
{
public:

	Mesh(Resource&& vertex_buffer, Resource&& index_buffer) noexcept
		:mVertexBuffer(std::move(vertex_buffer)), mIndexBuffer(std::move(index_buffer))
	{
	}

	void add_submesh(Submesh submesh)
	{
		mObjects.emplace_back(std::move(submesh));
	}

    void add_submesh(
        UINT64 vertex_offset,
        UINT vertex_size,
        UINT vertex_stride,
        UINT64 index_offset,
        UINT index_size,
        DXGI_FORMAT index_format
    )
    {
		mObjects.emplace_back
		(
			VertexBufferView(
				&mVertexBuffer,
				vertex_offset,
				vertex_size,
				vertex_stride
			),
			IndexBufferView(
				&mIndexBuffer,
				index_offset,
				index_size,
				index_format
			)
		);
    }

	const Resource* const vertex_resource() const noexcept { 
		return &mVertexBuffer; 
	}

	const Resource* const index_resource() const noexcept  { 
		return &mIndexBuffer; 
	
	}

	ID3D12Resource* const vertex_buffer() const noexcept { 
		return mVertexBuffer.get(); 
	}

	ID3D12Resource* const index_buffer() const noexcept { 
		return mIndexBuffer.get(); 
	}

	const std::vector<Submesh>& get_submeshes() const noexcept { 
		return mObjects; 
	}

	auto begin() const noexcept { 
		return mObjects.begin(); 
	}

	auto end() const noexcept { 
		return mObjects.end(); 
	}

private:
	Resource mVertexBuffer;
	Resource mIndexBuffer;

	std::vector<Submesh> mObjects;
};