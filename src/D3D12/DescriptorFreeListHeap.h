#pragma once

#include <mutex>
#include <map>
#include <queue>

#include <d3d12.h>
#include <wrl/client.h>

#include "D3D12/DescriptorHeap.h"
#include "D3D12/DescriptorRange.h"
#include "D3D12/FenceView.h"

class DescriptorFreeListHeap final
{
public:

	DescriptorFreeListHeap(const DescriptorFreeListHeap&) = delete;
	DescriptorFreeListHeap& operator=(const DescriptorFreeListHeap&) = delete;
	DescriptorFreeListHeap(DescriptorFreeListHeap&&) = delete;
	DescriptorFreeListHeap& operator=(DescriptorFreeListHeap&&) = delete;

	~DescriptorFreeListHeap() = default;

	// checks if there is a large enough contiguous block of descriptors to satisfy demand
	bool has_sapce(UINT size) const {
		return mSizeMap.lower_bound(size) != mSizeMap.end();
	}

	UINT count_of_free_elements() const noexcept {
		return mFreeCount;
	}

	// allocate a contiguous range of descriptors. if the allocation can not be satisfied a null descriptor is returned
	DescriptorRange allocate(UINT size);

	// frees an allocation. since the GPU may still be using the descriptors, actual release is deffered to release_stale_descriptors
	void free(D3D12_CPU_DESCRIPTOR_HANDLE handle, UINT size);

	// releases stale descriptors, if the GPU is done with the descriptors
	void release_stale_descriptors();

protected:

	friend class DescriptorAllocator;

	DescriptorFreeListHeap(
		ID3D12Device4* device,
		FenceView fence_view,
		D3D12_DESCRIPTOR_HEAP_TYPE type,
		UINT size_per_heap
	);

	// Compute the offset of the descriptor handle from the start of the heap.
	UINT compute_offset(D3D12_CPU_DESCRIPTOR_HANDLE handle) const noexcept {
		return static_cast<UINT>(handle.ptr - mD3D12DescriptorHeap.handle().ptr) / mD3D12DescriptorHeap.stride();
	}

	// Adds a new block to the free list.
	void add_new_block(UINT offset, UINT size);

	// frees the block of descriptors called by release_stale_descriptors
	void free_block(UINT offset, UINT size);

	using OffsetType = UINT;
	using SizeType = UINT;

	struct FreeBlockInfo;

	// a bidirectional map system: 
	// D3D12DescriptorHeap is a contiguous line. OffsetMap stores offsets as keys representing elements in the desc heap. Value represents the data of the block at the offset; the size of the block and a pointer to SizeMap
	//                                           SizeMap stores sizes as keys representing size of each block. There may be duplicate sizes. The Value represents a pointer to the OffsetMap.
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

	// Stale descriptors are required because the GPU may be executing some of the descriptors meaning the blocks may not be immediately freed
	// when called. instead they need to be stored (in this case a queue) to release explicitly calling release_stale_descriptors
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

private:

	// members
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
	std::mutex mMutex;
};