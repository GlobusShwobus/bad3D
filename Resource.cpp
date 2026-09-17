#include "Resource.h"
#include "EasyDirectX.h"
#include "Utils.h"

#include <assert.h>
#include <utility>

Resource Resource::create_commited(ID3D12Device4* device, const D3D12_HEAP_PROPERTIES& heap_properties, const D3D12_RESOURCE_DESC& resource_desc, D3D12_RESOURCE_STATES initial_state, D3D12_HEAP_FLAGS flags, const D3D12_CLEAR_VALUE* optimized_clear_value)
{
    assert(device && "nullptr");

    Microsoft::WRL::ComPtr<ID3D12Resource> resource;

    execute_and_test_hresult(
        device->CreateCommittedResource(
            &heap_properties,
            flags,
            &resource_desc,
            initial_state,
            optimized_clear_value,
            IID_PPV_ARGS(&resource)
        )
    );

    return Resource(std::move(resource), resource_desc, initial_state);
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

Resource::Resource(Microsoft::WRL::ComPtr<ID3D12Resource> resource, const D3D12_RESOURCE_DESC& desc, D3D12_RESOURCE_STATES state) noexcept
    : mResource(std::move(resource)), mDesc(desc), mState(state)
{
}

VertexBuffer::VertexBuffer(Resource resource, UINT element_type_size) noexcept
    : mResource(std::move(resource))
{
    assert(mResource.desc().Dimension == D3D12_RESOURCE_DIMENSION_BUFFER && "invalid buffer");

    mView = D3D12_VERTEX_BUFFER_VIEW{
        mResource.get()->GetGPUVirtualAddress(),
        static_cast<UINT>(mResource.desc().Width),
        static_cast<UINT>(element_type_size)
    };
}

IndexBuffer::IndexBuffer(Resource resource, DXGI_FORMAT format) noexcept
    : mResource(std::move(resource))
{
    assert(format == DXGI_FORMAT_R16_UINT || format == DXGI_FORMAT_R32_UINT && "invalid format");
    assert(mResource.desc().Dimension == D3D12_RESOURCE_DIMENSION_BUFFER && "invalid buffer");
    mView = D3D12_INDEX_BUFFER_VIEW{
         mResource.get()->GetGPUVirtualAddress(),
         static_cast<UINT>(mResource.desc().Width),
         format
    };
}