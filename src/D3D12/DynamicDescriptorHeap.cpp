#include "D3D12/DynamicDescriptorheap.h"

#include <stdexcept>

DescriptorAllocator::DescriptorAllocator(
	ID3D12Device4* device,
	FenceView fence_view, 
	D3D12_DESCRIPTOR_HEAP_TYPE type, 
	UINT size_per_heap
)
	:mHeapType(type)
	, mFenceView(fence_view)
	, mSizePerHeap(size_per_heap)
	, mDevice(device)
{
	if (!mDevice)
		throw std::invalid_argument("device nullptr");
}

DescriptorRange DescriptorAllocator::allocate(uint32_t size)
{
	std::lock_guard<std::mutex> lock(mAllocationMutex);

	DescriptorRange allocation;

	for (auto iter = mAvailableHeaps.begin(); iter != mAvailableHeaps.end(); ++iter)
	{
		DescriptorFreeListHeap* heap = mHeapPool[*iter].get();

		// try to allocate from the heap, it may fail if there are not enough available slots
		allocation = heap->allocate(size);

		// if no free handles are left, the heap is full --> remove the index from the set
		if (heap->count_of_free_elements() == 0)
			iter = mAvailableHeaps.erase(iter);

		// a valid alloc has been found
		if (!allocation.is_null())
			break;
	}

	// if no available heap could satisfy the requested number of descriptors
	if (allocation.is_null())
	{
		auto tempSize = mSizePerHeap;

		mSizePerHeap = max(mSizePerHeap, size);
		DescriptorFreeListHeap* heap = create_allocator_heap();

		mSizePerHeap = tempSize;

		allocation = heap->allocate(size);
	}

	return allocation;
}

void DescriptorAllocator::release_stale_descriptors()
{
	std::lock_guard<std::mutex>lock(mAllocationMutex);

	for (std::size_t i = 0; i < mHeapPool.size(); ++i)
	{
		auto* heap = mHeapPool[i].get();

		heap->release_stale_descriptors();

		if (heap->count_of_free_elements() > 0)
			mAvailableHeaps.insert(i);
	}
}

DescriptorFreeListHeap* DescriptorAllocator::create_allocator_heap()
{
	auto new_free_list_heap = std::unique_ptr<DescriptorFreeListHeap>(new DescriptorFreeListHeap(mDevice.get(), mFenceView, mHeapType, mSizePerHeap));

	DescriptorFreeListHeap* pHeap = new_free_list_heap.get();

	mHeapPool.emplace_back(std::move(new_free_list_heap));
	mAvailableHeaps.insert(mHeapPool.size() - 1);

	return pHeap;
}