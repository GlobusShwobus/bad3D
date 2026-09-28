#pragma once

#include <d3d12.h>
#include <wrl/client.h>

#include "D3D12/Resource.h"

class VertexBufferView
{
public:
    VertexBufferView() noexcept :mView{ 0ull,0u,0u } {}

    VertexBufferView(const Resource& resource, UINT64 byte_offset, UINT byte_size, UINT stride) noexcept
        :mView{0ull,0u,0u}
    {
        const D3D12_RESOURCE_DESC desc = resource.desc();

        const bool dimension_correct = desc.Dimension == D3D12_RESOURCE_DIMENSION_BUFFER;
        const bool alignment_correct = stride != 0 && byte_size % stride == 0;
        const bool range_correct = byte_offset + byte_size <= desc.Width;

        if (dimension_correct && alignment_correct && range_correct)
        {
            mView = D3D12_VERTEX_BUFFER_VIEW{
                resource.get()->GetGPUVirtualAddress() + byte_offset,
                byte_size,
                stride
            };
        }
    }

    inline UINT stride_in_bytes() const noexcept { return mView.StrideInBytes; }
    inline UINT size_in_bytes() const noexcept { return mView.SizeInBytes; }
    inline UINT element_count() const noexcept { return mView.SizeInBytes / mView.StrideInBytes; }
    inline const D3D12_VERTEX_BUFFER_VIEW& view() const noexcept { return mView; }
private:
    D3D12_VERTEX_BUFFER_VIEW mView;
};

class IndexBufferView
{
public:
    IndexBufferView() noexcept :mView{ 0ull, 0, DXGI_FORMAT_UNKNOWN } {}

    IndexBufferView(const Resource& resource, UINT64 byte_offset, UINT byte_size, DXGI_FORMAT format) noexcept
        :mView{ 0ull, 0, DXGI_FORMAT_UNKNOWN }
    {
        const D3D12_RESOURCE_DESC desc = resource.desc();
        
        const bool dimension_correct = desc.Dimension == D3D12_RESOURCE_DIMENSION_BUFFER;
        const bool format_correct = format == DXGI_FORMAT_R16_UINT || format == DXGI_FORMAT_R32_UINT;
        
        if (dimension_correct && format_correct)
        {
            const UINT stride = index_stride_from_format(format);
            const bool alignment_correct = byte_size % stride == 0;
            const bool range_correct = byte_offset + byte_size <= desc.Width;

            if (alignment_correct && range_correct)
            {
                mView = D3D12_INDEX_BUFFER_VIEW{
                     resource.get()->GetGPUVirtualAddress() + byte_offset,
                     byte_size,
                     format
                };
            }
        }
    }

    inline DXGI_FORMAT format() const noexcept { return mView.Format; }
    inline UINT size_in_bytes() const noexcept { return mView.SizeInBytes; }
    inline UINT element_count() const noexcept {
        return mView.SizeInBytes / index_stride_from_format(mView.Format);
    }

    inline const D3D12_INDEX_BUFFER_VIEW& view() const noexcept { return mView; }

protected:

    constexpr UINT index_stride_from_format(DXGI_FORMAT format) const noexcept
    {
        return format == DXGI_FORMAT_R16_UINT ? 2u : 4u;
    }

private:
    D3D12_INDEX_BUFFER_VIEW mView;
};