#pragma once

#include <stdexcept>

#include <d3d12.h>
#include <wrl/client.h>

class IndexBufferView
{
public:

    constexpr IndexBufferView() noexcept 
        :mView{ 0ull, 0, DXGI_FORMAT_R16_UINT }
    {}

    constexpr IndexBufferView(D3D12_INDEX_BUFFER_VIEW view) noexcept
        :mView(view) 
    {}

    IndexBufferView(UINT64 resource_size, D3D12_GPU_VIRTUAL_ADDRESS address, UINT64 byte_offset, UINT byte_size, DXGI_FORMAT format)
    {
        if (format != DXGI_FORMAT_R16_UINT && format != DXGI_FORMAT_R32_UINT)
            throw std::invalid_argument{ "invalid format for an index buffer" };

        const UINT stride = index_stride_from_format(format);

        if (stride == 0 || byte_size % stride != 0)
            throw std::invalid_argument{ "bad stride/size" };

        if (byte_offset + byte_size > resource_size)
            throw std::out_of_range{ "view exceeds buffer" };

        mView = { address + byte_offset, byte_size, format };
    }

    DXGI_FORMAT format() const noexcept { 
        return mView.Format; 
    }

    UINT size_in_bytes() const noexcept {
        return mView.SizeInBytes;
    }

    UINT element_count() const noexcept {
        return mView.SizeInBytes / index_stride_from_format(mView.Format);
    }

    const D3D12_INDEX_BUFFER_VIEW& view() const noexcept {
        return mView; 
    }

protected:

    constexpr UINT index_stride_from_format(DXGI_FORMAT format) const noexcept {
        return format == DXGI_FORMAT_R16_UINT ? 2u : 4u;
    }

private:
    D3D12_INDEX_BUFFER_VIEW mView;
};