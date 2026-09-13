#pragma once

#include <d3d12.h>
#include <wrl/client.h>

class CommittedResource
{
public:
    //TODO: constructors
    CommittedResource() = default;

    Microsoft::WRL::ComPtr<ID3D12Resource> load(
        ID3D12Device4* device,
        ID3D12GraphicsCommandList2* cl,
        const void* data, UINT64 element_count, UINT64 type_size);


    ID3D12Resource* get() const noexcept;

    constexpr UINT64 count() const noexcept
    {
        return mCount;
    }

    constexpr UINT64 stride() const noexcept
    {
        return mStride;
    }

private:
    Microsoft::WRL::ComPtr<ID3D12Resource> mResource;
    UINT64                            mCount = 0;
    UINT64                            mStride = 0;
};