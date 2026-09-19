#include "Model/ModelBuffer.h"

#include <assert.h>

#include "D3D12/EasyDirectX.h"

VertexBuffer::VertexBuffer(ID3D12Device4* device, UINT64 byte_size, UINT element_type_size)
{
    mResource = Resource{
            device,
            easy::heap_property_default(),
            easy::resource_desc_buffer(byte_size),
            D3D12_RESOURCE_STATE_COMMON
    };

    mView = D3D12_VERTEX_BUFFER_VIEW{
        mResource.get()->GetGPUVirtualAddress(),
        static_cast<UINT>(byte_size), // why the fuck does id3d12 need UINT64 for desc but UINT for size???
        element_type_size
    };
}

D3D12_VERTEX_BUFFER_VIEW VertexBuffer::create_subview(UINT64 byte_position, UINT byte_count) const noexcept
{
    D3D12_VERTEX_BUFFER_VIEW subview = {};

    if ((byte_position + byte_count) <= mView.SizeInBytes)
    {
        subview.BufferLocation = mView.BufferLocation + byte_position;
        subview.SizeInBytes = byte_count;
        subview.StrideInBytes = mView.StrideInBytes;
    }

    return subview;
}

IndexBuffer::IndexBuffer(ID3D12Device4* device, UINT64 byte_size, DXGI_FORMAT format)
{
    assert(format == DXGI_FORMAT_R16_UINT || format == DXGI_FORMAT_R32_UINT && "invalid format");

    mResource = Resource{
        device,
        easy::heap_property_default(),
        easy::resource_desc_buffer(byte_size),
        D3D12_RESOURCE_STATE_COMMON
    };

    mView = D3D12_INDEX_BUFFER_VIEW{
         mResource.get()->GetGPUVirtualAddress(),
         static_cast<UINT>(byte_size),
         format
    };
}

D3D12_INDEX_BUFFER_VIEW IndexBuffer::create_subview(UINT64 byte_position, UINT byte_count) const noexcept
{
    D3D12_INDEX_BUFFER_VIEW subview = {};

    if ((byte_position + byte_count) <= mView.SizeInBytes)
    {
        subview.BufferLocation = mView.BufferLocation + byte_position;
        subview.SizeInBytes = byte_count;
        subview.Format = mView.Format;
    }

    return subview;
}