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