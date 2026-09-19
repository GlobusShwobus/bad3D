#include "D3D12/Resource.h"

#include <assert.h>

#include "D3D12/EasyDirectX.h"
#include "D3D12/EasyDirectXUtils.h"

Resource::Resource(ID3D12Device4* device, const D3D12_HEAP_PROPERTIES& heap_properties, const D3D12_RESOURCE_DESC& resource_desc, D3D12_RESOURCE_STATES initial_state, D3D12_HEAP_FLAGS flags, const D3D12_CLEAR_VALUE* optimized_clear_value)
    :mState(initial_state)
{
    assert(device && "nullptr");

    mResource = create_commited_resource(
        device,
        heap_properties,
        resource_desc,
        initial_state,
        flags,
        optimized_clear_value
    );

    assert(mResource && "nullptr");
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