#pragma once

#include <utility>
#include <vector>

#include <d3d12.h>
#include <wrl/client.h>

#include "D3D12/GPUResource.h"
#include "D3D12/IndexView.h"
#include "D3D12/VertexView.h"

struct Submesh
{
	VertexBufferView vertex_view;
	IndexBufferView index_view;
};

// as things go ideally shit is not as exposed

// calling get desck in add submesh for resource size. resource size cache somehow??? probably IN mesh class because

class Mesh // mesh for textured objects????? idk
{
public:

	Mesh(GPUResource&& vertex_buffer, GPUResource&& index_buffer) noexcept
		:mVertexBuffer(std::move(vertex_buffer)), mIndexBuffer(std::move(index_buffer)), mVBufSize(0), mIBufSize(0)
	{
		const auto vertex_desc = mVertexBuffer->GetDesc();
		const auto index_desc = mIndexBuffer->GetDesc();

		mVBufSize = vertex_desc.Width;
		mIBufSize = index_desc.Width;
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
				mVBufSize,
				mVertexBuffer.address(),
				vertex_offset,
				vertex_size,
				vertex_stride
			),
			IndexBufferView(
				mIBufSize,
				mIndexBuffer.address(),
				index_offset,
				index_size,
				index_format
			)
		);
    }

	const GPUResource* const vertex_resource() const noexcept {
		return &mVertexBuffer; 
	}

	const GPUResource* const index_resource() const noexcept  {
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
	GPUResource mVertexBuffer;
	GPUResource mIndexBuffer;

	UINT64 mVBufSize;
	UINT64 mIBufSize;

	std::vector<Submesh> mObjects;
};