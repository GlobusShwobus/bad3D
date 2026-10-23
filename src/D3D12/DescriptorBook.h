#pragma once

#include <memory>
#include <mutex>
#include <set>
#include <vector>

#include <d3d12.h>

#include "D3D12/DescriptorAlloc.h"
#include "D3D12/DescriptorPage.h"
#include "D3D12/FenceView.h"
#include "Tools/ViewPtr.h"

/**
* DynamicDescriptorHeap public methods are thread safe.
*/

class DescriptorBook
{
public:

	DescriptorBook(
		ID3D12Device4* device,            // device for creation
		FenceView fence_view,             // fence value for frame sync
		D3D12_DESCRIPTOR_HEAP_TYPE type,  // type of heap
		UINT size_per_heap = 256          // size of heap
	);

	DescriptorBook(const DescriptorBook&) = delete;
	DescriptorBook& operator=(const DescriptorBook&) = delete;

	DescriptorBook(DescriptorBook&&)noexcept = default;
	DescriptorBook& operator=(DescriptorBook&&)noexcept = default;

	virtual ~DescriptorBook() = default;

	// allocate a number of contiguous descriptors from a CPU visible descriptor heap
	DescriptorAlloc allocate_descriptors(UINT size = 1);

	// when the frame has completed, the stale descriptors can be released
	void release_stale_descriptors();

protected:

	// create a new free list heap
	DescriptorPage* create_allocator_heap();

private:

	std::vector<std::unique_ptr<DescriptorPage>> mHeapPool;
	D3D12_DESCRIPTOR_HEAP_TYPE mHeapType;
	UINT mSizePerHeap;

	ViewPtr<ID3D12Device4> mDevice;

	// indices of available heaps in the heap pool
	std::set<UINT64> mAvailableHeaps;

	FenceView mFenceView;
	std::mutex mLock;
};