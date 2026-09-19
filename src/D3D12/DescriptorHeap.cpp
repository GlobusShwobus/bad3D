#include "D3D12/DescriptorHeap.h"

#include <assert.h>

#include "D3D12/EasyDirectXUtils.h"

DescriptorHeap::DescriptorHeap(ID3D12Device4* device, const D3D12_DESCRIPTOR_HEAP_DESC& desc)
	:mType(desc.Type), mCount(desc.NumDescriptors)
{
	assert(device && "device nullptr");
	
	execute_and_test_hresult(
		device->CreateDescriptorHeap(&desc, IID_PPV_ARGS(&mHeap))
	);

	mStride = device->GetDescriptorHandleIncrementSize(mType);
	mBegin = mHeap->GetCPUDescriptorHandleForHeapStart();
}

D3D12_CPU_DESCRIPTOR_HANDLE DescriptorHeap::descriptor_at(SIZE_T index) const noexcept
{
	assert(mHeap && index < mCount);
	// pointer arithmetic, offset from begin to index times size in bytes
	return D3D12_CPU_DESCRIPTOR_HANDLE{ mBegin.ptr + index * mStride };
}
