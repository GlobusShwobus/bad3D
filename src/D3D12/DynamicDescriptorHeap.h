#pragma once

#include <memory>
#include <mutex>
#include <set>
#include <vector>

#include <d3d12.h>

#include "D3D12/DescriptorRange.h"
#include "D3D12/DescriptorFreeListHeap.h"
#include "D3D12/FenceView.h"
#include "Tools/ViewPtr.h"

class DescriptorAllocator
{
public:

	DescriptorAllocator(ID3D12Device4* device,
		FenceView fence_view,
		D3D12_DESCRIPTOR_HEAP_TYPE type,
		UINT size_per_heap = 256
	);

	DescriptorAllocator(const DescriptorAllocator&) = delete;
	DescriptorAllocator& operator=(const DescriptorAllocator&) = delete;

	DescriptorAllocator(DescriptorAllocator&&)noexcept = default;
	DescriptorAllocator& operator=(DescriptorAllocator&&)noexcept = default;

	virtual ~DescriptorAllocator() = default;

	// allocate a number of contiguous descriptors from a CPU visible descriptor heap
	DescriptorRange allocate(UINT size = 1);

	// when the frame has completed, the stale descriptors can be released
	void release_stale_descriptors();

private:

	// create a new free list heap
	DescriptorFreeListHeap* create_allocator_heap();

	std::vector<std::unique_ptr<DescriptorFreeListHeap>> mHeapPool;
	D3D12_DESCRIPTOR_HEAP_TYPE mHeapType;
	UINT mSizePerHeap;

	ViewPtr<ID3D12Device4> mDevice;

	// indices of available heaps in the heap pool
	std::set<UINT64> mAvailableHeaps;

	FenceView mFenceView;
	std::mutex mAllocationMutex;
};