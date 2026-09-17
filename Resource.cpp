#include "Resource.h"
#include "EasyDirectX.h"
#include "Utils.h"

#include <assert.h>
#include <utility>

Resource::Resource(ID3D12Device4* device, const D3D12_HEAP_PROPERTIES& heap_properties, const D3D12_RESOURCE_DESC& resource_desc, D3D12_RESOURCE_STATES initial_state, D3D12_HEAP_FLAGS flags, const D3D12_CLEAR_VALUE* optimized_clear_value)
    :mState(initial_state)
{
    assert(device && "nullptr");

    execute_and_test_hresult(
        device->CreateCommittedResource(
            &heap_properties,
            flags,
            &resource_desc,
            initial_state,
            optimized_clear_value,
            IID_PPV_ARGS(&mResource)
        )
    );
}

D3D12_RESOURCE_DESC Resource::desc() const
{
    return mResource->GetDesc();
}

D3D12_RESOURCE_STATES Resource::exchange_state(D3D12_RESOURCE_STATES after) noexcept
{
    D3D12_RESOURCE_STATES state = mState;
    mState = after;
    return state;
}

void Resource::transition_state(ID3D12GraphicsCommandList2* list, D3D12_RESOURCE_STATES after, D3D12_RESOURCE_BARRIER_FLAGS flags, UINT sub_resource)
{
    if (mState == after)
        return;

    auto barrier = RESOURCE_BARRIER::transition(mResource.Get(), mState , after, flags, sub_resource);

    list->ResourceBarrier(1, &barrier);

    mState = after;
}

VertexBuffer::VertexBuffer(ID3D12Device4* device, UINT64 byte_size, UINT element_type_size)
{
    mResource = Resource{
            device,
            HEAP_PROPERTY::base(),
            RESOURCE_DESC::buffer(byte_size),
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

IndexBuffer::IndexBuffer(ID3D12Device4* device, UINT64 byte_size, DXGI_FORMAT format) noexcept
{
    assert(format == DXGI_FORMAT_R16_UINT || format == DXGI_FORMAT_R32_UINT && "invalid format");

    mResource = Resource{
        device,
        HEAP_PROPERTY::base(),
        RESOURCE_DESC::buffer(byte_size),
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