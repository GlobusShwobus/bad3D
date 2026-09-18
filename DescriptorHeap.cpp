#include "DescriptorHeap.h"
#include "EasyDirectX.h"
#include "EasyDirectXUtils.h"
#include <assert.h>

DescriptorHeap::DescriptorHeap(ID3D12Device4* device, UINT desc_count, D3D12_DESCRIPTOR_HEAP_TYPE type, D3D12_DESCRIPTOR_HEAP_FLAGS flags, UINT node_masks)
	:mType(type), mCount(desc_count)
{
	assert(device && "device nullptr");
	
	D3D12_DESCRIPTOR_HEAP_DESC desc = DESCRIPTOR_HEAP_DESC::custom(type, desc_count, flags, node_masks);

	execute_and_test_hresult(
		device->CreateDescriptorHeap(&desc, IID_PPV_ARGS(&mHeap))
	);

	mStride = device->GetDescriptorHandleIncrementSize(type);
	mBegin = mHeap->GetCPUDescriptorHandleForHeapStart();
}

D3D12_CPU_DESCRIPTOR_HANDLE DescriptorHeap::descriptor_at(SIZE_T index) const noexcept
{
	assert(mHeap && index < mCount);
	// pointer arithmetic, offset from begin to index times size in bytes
	return D3D12_CPU_DESCRIPTOR_HANDLE{ mBegin.ptr + index * mStride };
}
