#include "DescriptorHeap.h"
#include "Utils.h"
#include "EasyDirectX.h"
#include <utility>

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

DescriptorHeap::DescriptorHeap(DescriptorHeap&& rhs) noexcept
	:mHeap(std::move(rhs.mHeap)), mStride(rhs.mStride), mCount(rhs.mCount), mType(rhs.mType), mBegin(rhs.mBegin)
{
	rhs.mStride = 0;
	rhs.mCount = 0;
	rhs.mBegin = {};
}

DescriptorHeap& DescriptorHeap::operator=(DescriptorHeap&& rhs) noexcept
{
	if (this != &rhs)
	{
		mHeap = std::move(rhs.mHeap);
		mStride = rhs.mStride;
		mCount = rhs.mCount;
		mType = rhs.mType;
		mBegin = rhs.mBegin;

		rhs.mStride = 0;
		rhs.mCount = 0;
		rhs.mBegin = {};
	}
	return *this;
}

D3D12_CPU_DESCRIPTOR_HANDLE DescriptorHeap::descriptor_at(SIZE_T index) const noexcept
{
	assert(mHeap && index < mCount);

	D3D12_CPU_DESCRIPTOR_HANDLE handle = { 0 };
	// pointer arithmetic, offset from begin to index times size in bytes
	handle.ptr = mBegin.ptr + index * mStride;

	return handle;
}
