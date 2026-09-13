#include "Resource.h"
#include "EasyDirectX.h"
#include "EasyDirectXUtils.h"

Microsoft::WRL::ComPtr<ID3D12Resource> CommittedResource::load(ID3D12Device4* device, ID3D12GraphicsCommandList2* cl, const void* data, UINT64 element_count, UINT64 type_size)
{
    const SIZE_T buffer_size = element_count * type_size;

    mResource = create_commited_resource(
        device,
        HEAP_PROPERTY::base(),
        RESOURCE_DESC::buffer(buffer_size),
        D3D12_RESOURCE_STATE_COMMON
    );

    auto intermediary = copy_buffer_to_resource_and_get_intermediary(
        device,
        cl,
        mResource.Get(),
        data,
        element_count,
        type_size
    );

    mCount = element_count;
    mStride = type_size;

    return intermediary;
}

ID3D12Resource* CommittedResource::get() const noexcept
{
    return mResource.Get();
}
