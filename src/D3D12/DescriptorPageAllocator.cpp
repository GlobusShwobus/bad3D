#include "D3D12/DescriptorPageAllocator.h"

#include <stdexcept>

DescriptorPageAllocator::DescriptorPage::DescriptorPage(
	ID3D12Device4* device,
	UINT size,
	D3D12_DESCRIPTOR_HEAP_TYPE type,
	D3D12_DESCRIPTOR_HEAP_FLAGS flags,
	UINT node_mask
)
	:mHeap(device, D3D12_DESCRIPTOR_HEAP_DESC{type, size, flags, node_mask})
	, mFreeDescCount(size)
{
	// if DescriptorHeap constructor didn't throw then we're gucci
	// initalize the free list
	add_new_block(0, mFreeDescCount);
}

bool DescriptorPageAllocator::DescriptorPage::create_descriptor_block(UINT requested_size, UINT& offset_out)
{
	// if the demand is too big to satisfy, return false
	if (requested_size > mFreeDescCount)
		return false;

	// find the first contiguous block that satisfies the size, if no such block exists return false
	const auto block = mSizeMap.lower_bound(requested_size);
	if (block == mSizeMap.end())
		return false;

	// The fitting block may be larger than the requested amount which means the fitting block must be split. 
	// The fitting block will be removed from the bimap and a resized one reinserted. For this some values need caching.
	const auto erased_block_size = block->first;
	const auto offset_map_it = block->second;
	const auto erased_block_offset = offset_map_it->first;

	mSizeMap.erase(block);
	mOffsetMap.erase(offset_map_it);

	// Compute the resized block offset and size and reinsert it if size is not 0.
	const auto reinserted_block_offset = erased_block_offset + requested_size;
	const auto reinserted_block_size = erased_block_size - requested_size;

	if (reinserted_block_size > 0)
		add_new_block(reinserted_block_offset, reinserted_block_size);

	// decrement free handles counter and return a valid range
	mFreeDescCount -= requested_size;
	
	offset_out = erased_block_offset;

	return true;
}

void DescriptorPageAllocator::DescriptorPage::free_descriptor_block(UINT offset, UINT size)
{
	// look for the next free block
	auto next_block = mOffsetMap.upper_bound(offset);

	// look for the previous block
	auto prev_block = next_block != mOffsetMap.begin() ? std::prev(next_block) : mOffsetMap.end();

	auto resized_block_offset = offset;
	auto resized_block_size = size;

	// It's not enough to just free a block back to the list. The blocks must be contiguous which means merging of neighboring blocks is required.
	// ALL POSSIBILITIES:
	// 
	// PrevBlock.Offset           Freed Offset         NextBlock.Offset 
	// |                          |                    |
	// |<-----PrevBlock.Size----->|<------Size-------->|<-----NextBlock.Size----->|

	// CASE 1: is prev block immediately before freed block
	if (prev_block != mOffsetMap.end() && (resized_block_offset == prev_block->first + prev_block->second.size))
	{
		// increase the block size by merging previous block (set offset to prev block offset and add previous block size on top)
		resized_block_offset = prev_block->first;
		resized_block_size += prev_block->second.size;

		// remove the previous block from the free list
		mSizeMap.erase(prev_block->second.size_map_iterator);
		mOffsetMap.erase(prev_block);
	}

	// CASE 2: is next block immediately after freed block
	if (next_block != mOffsetMap.end() && (resized_block_offset + resized_block_size == next_block->first))
	{
		// increase the block size by the size of the next block (no offset)
		resized_block_size += next_block->second.size;

		// remove the next block from the free list
		mSizeMap.erase(next_block->second.size_map_iterator);
		mOffsetMap.erase(next_block);
	}

	// add the size of the original free block back to the count
	mFreeDescCount += size;

	// add the freed and resized block into the free list
	add_new_block(resized_block_offset, resized_block_size);
}

void DescriptorPageAllocator::DescriptorPage::add_new_block(UINT offset, UINT size)
{
	// make a bidirectional view
	auto offsetIt = mOffsetMap.emplace(offset, size);
	auto sizeIt = mSizeMap.emplace(size, offsetIt.first);
	offsetIt.first->second.size_map_iterator = sizeIt;
}
//########################################################################################################
//########################################################################################################
//########################################################################################################
//########################################################################################################

DescriptorPageAllocator::DescriptorPageAllocator(
	ID3D12Device4* device, 
	FenceView fence_view,
	D3D12_DESCRIPTOR_HEAP_DESC heap_desc)

	:mDevice(device)
	, mHeapDesc(heap_desc)
	, mFenceView(fence_view)
{
	if (!mDevice)
		throw std::invalid_argument("device nullptr");

	assert(mHeapDesc.NumDescriptors > 0);
	assert(!(mHeapDesc.Flags & D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE));
}

DescriptorAlloc DescriptorPageAllocator::create_descriptor_allocation(UINT requested_size)
{
	assert(requested_size > 0);

	std::lock_guard<std::mutex> lock(mAllocLock);

	for (auto iter = mFreePages.begin(); iter != mFreePages.end();)
	{
		const UINT index = static_cast<UINT>(*iter);
		auto* page = mPagePool[index].get();

		// try to create a desc block, if success break away from the loop
		UINT offset = 0;
		if (page->create_descriptor_block(requested_size, offset))
		{
			// this check is not required but i leave it to conceptually show what is required, this also applies if the loop failed
			// to create a block, then on the next call to create_descriptor_allocation, the clean up is done
			// if (page->count_of_free_descriptors() == 0)
			//	 mFreePages.erase(iter);

			return DescriptorAlloc(page->handle(offset), offset, requested_size, page->stride(), this, index);
		}

		if (page->count_of_free_descriptors() == 0)
			iter = mFreePages.erase(iter);
		else
			++iter;
	}

	// if the loop did not return a desc, alternative is invoked
	const UINT index = static_cast<UINT>(
		create_page(mHeapDesc.NumDescriptors > requested_size ? mHeapDesc.NumDescriptors : requested_size)
		);

	DescriptorPage& page = *mPagePool[index];
	
	UINT offset = 0;
	if (page.create_descriptor_block(requested_size, offset))
		return DescriptorAlloc(page.handle(offset), offset, requested_size, page.stride(), this, index);

	// null desc if all else failed
	return DescriptorAlloc{};
}

void DescriptorPageAllocator::free_descriptor_allocation(UINT index, UINT offset, UINT size)
{
	std::lock_guard lock(mStaleLock);

	mStaleDescriptors.emplace(
		index,
		offset,
		size,
		mFenceView.get_next_signal_value()
	);
}

void DescriptorPageAllocator::release_stale_descriptors()
{
	// pahse 1: copy out the stale descriptors out of the queue
	//			no more stales may be inserted during this op, BUT allocs can be
	std::vector<StaleDescriptor> ready;
	{
		std::lock_guard lock(mStaleLock);
		
		const auto completed = mFenceView.get_completed_value();

		while (!mStaleDescriptors.empty() && mStaleDescriptors.front().fence_value <= completed)
		{
			ready.push_back(mStaleDescriptors.front());
			mStaleDescriptors.pop();
		}
	}

	// no op
	if (ready.empty())
		return;

	//phase 2: free blocks in all pages, locking the page before doing it. 
	//		   at this point more stales may be inserted but no allocs may be made
	std::lock_guard lock(mAllocLock);

	for (const auto& s : ready) 
	{
		assert(s.index < mPagePool.size());
		mPagePool[s.index]->free_descriptor_block(s.offset, s.size);
		mFreePages.insert(s.index);

	}
}

UINT64 DescriptorPageAllocator::create_page(UINT size)
{
	auto new_free_list_heap = std::make_unique<DescriptorPage>(mDevice.get(), size, mHeapDesc.Type, mHeapDesc.Flags, mHeapDesc.NodeMask);

	mPagePool.emplace_back(std::move(new_free_list_heap));

	auto index = mPagePool.size() - 1;
	mFreePages.insert(index);

	return index;
}
