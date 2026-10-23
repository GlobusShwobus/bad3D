#include "D3D12/DescriptorPage.h"

DescriptorPage::DescriptorPage(
	ID3D12Device4* device,
	FenceView fence_view,
	D3D12_DESCRIPTOR_HEAP_TYPE type,
	UINT size_per_heap)
	:mD3D12DescriptorHeap(device, D3D12_DESCRIPTOR_HEAP_DESC{ type, size_per_heap, D3D12_DESCRIPTOR_HEAP_FLAG_NONE, 0u })
	, mFenceView(fence_view)
	, mFreeCount(size_per_heap)
{
	// if DescriptorHeap constructor didn't throw then we're gucci
	// initalize the free list
	add_new_block(0, size_per_heap);
}

DescriptorAlloc DescriptorPage::freelist_allocate(UINT size)
{
	std::lock_guard<std::mutex> lock(mMutex);

	// if the demand is too big to satisfy, return null descriptor
	if (size > mFreeCount)
		return DescriptorAlloc();

	// find the first contiguous block that satisfies the size, if no such block exists return null descriptor
	// (same as has_space() but the iterator is needed)
	const auto fitting_block = mSizeMap.lower_bound(size);
	if (fitting_block == mSizeMap.end())
		return DescriptorAlloc();

	// The fitting block may be larger than the requested amount which means the fitting block must be split. 
	// The fitting block will be removed from the bimap and a resized one reinserted. For this some values need caching.
	const auto old_block_size = fitting_block->first;
	const auto fitting_block_offset = fitting_block->second;
	const auto old_block_offset = fitting_block_offset->first; 

	mSizeMap.erase(fitting_block);
	mOffsetMap.erase(fitting_block_offset);

	// Compute the resized block offset and size and reinsert it if size is not 0.
	const auto reinserted_block_offset = old_block_offset + size;
	const auto reinserted_block_size = old_block_size - size;   

	if (reinserted_block_size > 0)
		add_new_block(reinserted_block_offset, reinserted_block_size);

	// decrement free handles counter and return a valid range
	mFreeCount -= size;

	const auto allocation_offset = old_block_offset;
	const auto allocation_size = size;

	return DescriptorAlloc(
		mD3D12DescriptorHeap.handle(allocation_offset),
		allocation_size,
		mD3D12DescriptorHeap.stride(),
		this
	);
}

void DescriptorPage::free(D3D12_CPU_DESCRIPTOR_HANDLE handle, UINT size)
{
	const auto offset = compute_offset(handle);

	std::lock_guard<std::mutex>lock(mMutex);

	mStaleDescriptors.emplace(offset, size, mFenceView.get_next_signal_value());
}

void DescriptorPage::release_stale_descriptors()
{
	std::lock_guard<std::mutex> lock(mMutex);

	const auto completed_value = mFenceView.get_completed_value();

	while (!mStaleDescriptors.empty())
	{
		const auto& stale_desc = mStaleDescriptors.front();
		// since the container is a queue, first in first out will start from the earlist entries with earliest fence values
		if (stale_desc.FenceValue > completed_value)
			break;

		const auto offset = stale_desc.Offset;
		const auto count = stale_desc.Size;

		free_block(offset, count);

		mStaleDescriptors.pop();
	}
}

void DescriptorPage::add_new_block(UINT offset, UINT size)
{
	// make a bidirectional view
	auto offsetIt = mOffsetMap.emplace(offset, size);
	auto sizeIt = mSizeMap.emplace(size, offsetIt.first);
	offsetIt.first->second.FreeListBySizeIt = sizeIt;
}

void DescriptorPage::free_block(UINT offset, UINT size)
{
	// find the next free block in the offset list
	auto nextBlockIt = mOffsetMap.upper_bound(offset);

	// find the block that appears before the block being freed. if it is not the first block go to the previous block
	// otherwise set it to end iterator to skip one of the cases
	auto prevBlockIt = nextBlockIt;

	if (prevBlockIt != mOffsetMap.begin())
		--prevBlockIt;
	else
		prevBlockIt = mOffsetMap.end();

	auto resized_block_offset = offset;
	auto resized_block_size = size;

	// It's not enough to just free a block back to the list. The blocks must be contiguous which means merging of neighboring blocks is required.
	// ALL POSSIBILITIES:
	// 
	// PrevBlock.Offset           Freed Offset         NextBlock.Offset 
	// |                          |                    |
	// |<-----PrevBlock.Size----->|<------Size-------->|<-----NextBlock.Size----->|

	// CASE 1: the previous block is immediately preceding the block being freed
	if (prevBlockIt != mOffsetMap.end() && resized_block_offset == prevBlockIt->first + prevBlockIt->second.Size)
	{
		// increase the block size by the size of merging with the previous block
		resized_block_offset = prevBlockIt->first;
		resized_block_size += prevBlockIt->second.Size;

		// remove the previous block from the free list
		mSizeMap.erase(prevBlockIt->second.FreeListBySizeIt);
		mOffsetMap.erase(prevBlockIt);
	}

	// CASE 2: if the next block is immediately after the block being freed
	if (nextBlockIt != mOffsetMap.end() && resized_block_offset + resized_block_size == nextBlockIt->first)
	{
		// increase the block size by the size of the merging with the next block
		resized_block_size += nextBlockIt->second.Size;

		// remove the next block from the free list
		mSizeMap.erase(nextBlockIt->second.FreeListBySizeIt);
		mOffsetMap.erase(nextBlockIt);
	}

	// add the size of the original free block back to the count
	mFreeCount += size;

	// add the freed and resized block into the free list
	add_new_block(resized_block_offset, resized_block_size);
}