#pragma once

#include <d3d12.h>
#include <wrl/client.h>

#include "D3D12/Resource.h"

class VertexBuffer
{
public:
    VertexBuffer() = default;
    VertexBuffer(ID3D12Device4* device, UINT64 byte_size, UINT element_type_size);

    ID3D12Resource* get() const noexcept { return mResource.get(); }
    Resource& resource() noexcept { return mResource; }

    constexpr UINT stride_in_bytes() const noexcept { return mView.StrideInBytes; }
    constexpr UINT size_in_bytes() const noexcept { return mView.SizeInBytes; }
    constexpr UINT element_count() const noexcept { return mView.SizeInBytes / mView.StrideInBytes; }

    const D3D12_VERTEX_BUFFER_VIEW& view() const noexcept { return mView; }
    D3D12_VERTEX_BUFFER_VIEW create_subview(UINT64 byte_position, UINT byte_count) const noexcept;
private:
    Resource mResource;
    D3D12_VERTEX_BUFFER_VIEW mView = {};
};

class IndexBuffer
{
public:
    IndexBuffer() = default;
    IndexBuffer(ID3D12Device4* device, UINT64 byte_size, DXGI_FORMAT format);

    ID3D12Resource* get() const noexcept { return mResource.get(); }
    Resource& resource() noexcept { return mResource; }

    static constexpr UINT index_stride_from_format(DXGI_FORMAT format) noexcept
    {
        return format == DXGI_FORMAT_R16_UINT ? 2u : 4u;
    }

    constexpr DXGI_FORMAT format() const noexcept { return mView.Format; }
    constexpr UINT size_in_bytes() const noexcept { return mView.SizeInBytes; }
    constexpr UINT element_count() const noexcept {
        return mView.SizeInBytes / index_stride_from_format(mView.Format);
    }

    const D3D12_INDEX_BUFFER_VIEW& view() const noexcept { return mView; }
    D3D12_INDEX_BUFFER_VIEW create_subview(UINT64 byte_position, UINT byte_count) const noexcept;

private:
    Resource mResource;
    D3D12_INDEX_BUFFER_VIEW mView = {};
};