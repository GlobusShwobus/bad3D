#pragma once

#include <memory>
#include <mutex>
#include <set>
#include <vector>
#include <map>
#include <queue>

#include <d3d12.h>

#include "D3D12/DescriptorAlloc.h"
#include "D3D12/DescriptorHeap.h"
#include "D3D12/FenceView.h"
#include "Tools/ViewPtr.h"

class DescriptorPage final
{
public:

	DescriptorPage(const DescriptorPage&) = delete;
	DescriptorPage& operator=(const DescriptorPage&) = delete;
	DescriptorPage(DescriptorPage&&) = delete;
	DescriptorPage& operator=(DescriptorPage&&) = delete;

	DescriptorPage(
		ID3D12Device4* device,
		FenceView fence_view,
		D3D12_DESCRIPTOR_HEAP_TYPE type,
		UINT size_per_heap
	);

	~DescriptorPage() = default;

	// checks if there is a large enough contiguous block of descriptors to satisfy demand
	bool has_space(UINT size) const {
		return mSizeMap.lower_bound(size) != mSizeMap.end();
	}

	// returns the number of total elements, including fragmentation
	UINT count_of_free_elements() const noexcept {
		return mFreeCount;
	}

	DescriptorAlloc freelist_allocate(UINT size); // not thread safe

	void free(D3D12_CPU_DESCRIPTOR_HANDLE handle, UINT size); // thread safe

	void release_stale_descriptors(); // not thread safe

protected:

	// Compute the offset of the descriptor handle from the start of the heap.
	UINT compute_offset(D3D12_CPU_DESCRIPTOR_HANDLE handle) const noexcept {
		return static_cast<UINT>(handle.ptr - mD3D12DescriptorHeap.handle().ptr) / mD3D12DescriptorHeap.stride();
	}

	void add_new_block(UINT offset, UINT size); // not thread safe

	void free_block(UINT offset, UINT size); // not thread safe

private:

	using OffsetType = UINT;
	using SizeType = UINT;

	struct FreeBlockInfo;

	using OffsetMap = std::map<OffsetType, FreeBlockInfo>;
	using SizeMap = std::multimap<SizeType, OffsetMap::iterator>;

	struct FreeBlockInfo
	{
		FreeBlockInfo(SizeType size)
			: Size(size)
		{
		}

		SizeType Size;
		SizeMap::iterator FreeListBySizeIt;
	};

	struct StaleDescriptorInfo
	{
		StaleDescriptorInfo(OffsetType offset, SizeType size, UINT64 fence_value)
			: Offset(offset)
			, Size(size)
			, FenceValue(fence_value)
		{
		}

		OffsetType Offset;
		SizeType Size;
		UINT64 FenceValue;
	};

	using StaleDescriptorQueue = std::queue<StaleDescriptorInfo>;


	DescriptorHeap mD3D12DescriptorHeap;

	// bidirectional mapping of free blocks
	OffsetMap mOffsetMap;
	SizeMap mSizeMap;

	// queue for deffered block releasing
	StaleDescriptorQueue mStaleDescriptors;

	// a count for checking if the desc heap has enough total space (not contiguous; for contiguous checks mSizeMap is used)
	UINT mFreeCount;

	// synchronization unit with the command queue
	FenceView mFenceView;

	// for locking in case of multithreading
	std::mutex mFreeLock;
};

class DescriptorPageAllocator
{
public:

	DescriptorPageAllocator(
		ID3D12Device4* device,            // device for creation
		FenceView fence_view,             // fence value for frame sync
		D3D12_DESCRIPTOR_HEAP_TYPE type,  // type of heap
		UINT size_per_heap = 256          // size of heap
	);

	DescriptorPageAllocator(const DescriptorPageAllocator&) = delete;
	DescriptorPageAllocator& operator=(const DescriptorPageAllocator&) = delete;

	DescriptorPageAllocator(DescriptorPageAllocator&&)noexcept = delete; // since mutex is not movable
	DescriptorPageAllocator& operator=(DescriptorPageAllocator&&)noexcept = delete;

	virtual ~DescriptorPageAllocator() = default;

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