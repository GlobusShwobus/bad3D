#pragma once

#include <stdexcept>

#include <d3d12.h>
#include <wrl/client.h>

class VertexBufferView
{
public:

    constexpr VertexBufferView() noexcept
        :mView{ 0ull,0u,0u }
    {
    }
    constexpr VertexBufferView(D3D12_VERTEX_BUFFER_VIEW view) noexcept
        :mView(view)
    {
    }

    VertexBufferView( UINT64 resource_size, D3D12_GPU_VIRTUAL_ADDRESS address, UINT64 byte_offset, UINT byte_size, UINT stride )
    {
        if (stride == 0 || byte_size % stride != 0)
            throw std::invalid_argument{ "bad stride/size" };

        if (byte_offset + byte_size > resource_size)
            throw std::out_of_range{ "view exceeds buffer" };

        mView = { address + byte_offset, byte_size, stride };
    }

    UINT stride_in_bytes() const noexcept {
        return mView.StrideInBytes;
    }

    UINT size_in_bytes() const noexcept {
        return mView.SizeInBytes;
    }

    UINT element_count() const noexcept {
        return mView.SizeInBytes / mView.StrideInBytes;
    }

    const D3D12_VERTEX_BUFFER_VIEW& view() const noexcept {
        return mView;
    }

private:

    D3D12_VERTEX_BUFFER_VIEW mView;
};