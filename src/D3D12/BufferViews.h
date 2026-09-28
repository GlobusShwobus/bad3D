#pragma once

#include <d3d12.h>
#include <wrl/client.h>

#include "Tools/ViewPtr.h"
#include "D3D12/Resource.h"

class VertexBufferView
{
public:
    VertexBufferView() = delete;

    VertexBufferView(Resource* const resource, UINT64 byte_offset, UINT byte_size, UINT stride)
    {
        if (!resource)
            throw std::invalid_argument{ "null resource" };

        const auto& desc = resource->desc();

        if (desc.Dimension != D3D12_RESOURCE_DIMENSION_BUFFER)
            throw std::invalid_argument{ "not a buffer" };

        if (stride == 0 || byte_size % stride != 0)
            throw std::invalid_argument{ "bad stride/size" };

        if (byte_offset + byte_size > desc.Width)
            throw std::out_of_range{ "view exceeds buffer" };

        mResource = ViewPtr{ resource };
        mView = { resource->address() + byte_offset, byte_size, stride};
    }

    inline UINT stride_in_bytes() const noexcept { return mView.StrideInBytes; }
    inline UINT size_in_bytes() const noexcept { return mView.SizeInBytes; }
    inline UINT element_count() const noexcept { return mView.SizeInBytes / mView.StrideInBytes; }
    inline const D3D12_VERTEX_BUFFER_VIEW& view() const noexcept { return mView; }
    inline ID3D12Resource* resource() const noexcept { return mResource->get(); }
private:

    ViewPtr<Resource> mResource;
    D3D12_VERTEX_BUFFER_VIEW mView;
};

class IndexBufferView
{
public:
    IndexBufferView() = delete;

    IndexBufferView(Resource* const resource, UINT64 byte_offset, UINT byte_size, DXGI_FORMAT format)
    {
        if (!resource)
            throw std::invalid_argument{ "null resource" };

        const auto& desc = resource->desc();

        if (desc.Dimension != D3D12_RESOURCE_DIMENSION_BUFFER)
            throw std::invalid_argument{ "not a buffer" };

        if (format != DXGI_FORMAT_R16_UINT && format != DXGI_FORMAT_R32_UINT)
            throw std::invalid_argument{ "invalid format for an index buffer" };

        const UINT stride = index_stride_from_format(format);

        if (stride == 0 || byte_size % stride != 0)
            throw std::invalid_argument{ "bad stride/size" };

        if (byte_offset + byte_size > desc.Width)
            throw std::out_of_range{ "view exceeds buffer" };

        mResource = ViewPtr{ resource };
        mView = { resource->address() + byte_offset, byte_size, format };
    }

    inline DXGI_FORMAT format() const noexcept { return mView.Format; }
    inline UINT size_in_bytes() const noexcept { return mView.SizeInBytes; }
    inline UINT element_count() const noexcept {
        return mView.SizeInBytes / index_stride_from_format(mView.Format);
    }

    inline const D3D12_INDEX_BUFFER_VIEW& view() const noexcept { return mView; }
    inline ID3D12Resource* resource() const noexcept { return mResource->get(); }

protected:

    constexpr UINT index_stride_from_format(DXGI_FORMAT format) const noexcept
    {
        return format == DXGI_FORMAT_R16_UINT ? 2u : 4u;
    }

private:
    ViewPtr<Resource> mResource;
    D3D12_INDEX_BUFFER_VIEW mView;
};