#pragma once
#include "badDirectX.h"
#include "EasyDirectX.h"
#include "EasyDirectXUtils.h"
#include <wrl/client.h>

class CommittedResource
{
public:
    //TODO: constructors
    CommittedResource() = default;

    Microsoft::WRL::ComPtr<ID3D12Resource> load(
        ID3D12Device4* device,
        ID3D12GraphicsCommandList2* cl,
        const void* data, SIZE_T element_count, SIZE_T type_size);


    ID3D12Resource* get() const noexcept;

    constexpr SIZE_T count() const noexcept
    {
        return mCount;
    }

    constexpr SIZE_T stride() const noexcept
    {
        return mStride;
    }

private:
    Microsoft::WRL::ComPtr<ID3D12Resource> mResource;
    SIZE_T                                 mCount = 0;
    SIZE_T                                 mStride = 0;
};