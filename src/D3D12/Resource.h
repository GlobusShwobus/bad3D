#pragma once

#include <stdexcept>

#include <d3d12.h>
#include <wrl/client.h>

#include "D3D12/EasyDirectX.h"
#include "D3D12/EasyDirectXUtils.h"

class Resource
{
public:
    Resource() = delete;
    Resource(const Resource&) = delete;
    Resource& operator=(const Resource&) = delete;

    Resource(Resource&&) = default;
    Resource& operator=(Resource&&) = default;

    Resource(
        ID3D12Device4* device,
        const D3D12_HEAP_PROPERTIES& heap_properties,
        const D3D12_RESOURCE_DESC& resource_desc,
        D3D12_RESOURCE_STATES initial_state,
        D3D12_HEAP_FLAGS flags = D3D12_HEAP_FLAG_NONE,
        const D3D12_CLEAR_VALUE* optimized_clear_value = nullptr
    )
    {
        if (!device)
            throw std::invalid_argument("device nullptr");

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
        mAddress = mResource->GetGPUVirtualAddress();
        mState = initial_state;
        mDesc = resource_desc;
    }

    inline ID3D12Resource* get() const noexcept { return mResource.Get(); }
    inline const D3D12_RESOURCE_DESC& desc() const noexcept { return mDesc; }
    inline D3D12_GPU_VIRTUAL_ADDRESS address() const noexcept { return mAddress; }

    inline D3D12_RESOURCE_STATES exchange_state(D3D12_RESOURCE_STATES after) noexcept
    {
        D3D12_RESOURCE_STATES state = mState;
        mState = after;
        return state;
    }

    inline void transition_state(
        ID3D12GraphicsCommandList2* list,
        D3D12_RESOURCE_STATES after,
        D3D12_RESOURCE_BARRIER_FLAGS flags = D3D12_RESOURCE_BARRIER_FLAG_NONE,
        UINT sub_resource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES
    )
    {
        if (mState == after)
            return;

        auto barrier = easy::resource_barrier_transition(mResource.Get(), mState, after, flags, sub_resource);

        list->ResourceBarrier(1, &barrier);

        mState = after;
    }

private:
    Microsoft::WRL::ComPtr<ID3D12Resource> mResource;
    D3D12_RESOURCE_DESC mDesc;
    D3D12_GPU_VIRTUAL_ADDRESS mAddress;
    D3D12_RESOURCE_STATES mState;
};