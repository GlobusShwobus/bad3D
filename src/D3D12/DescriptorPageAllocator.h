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

// todo:: alloc class friend of this class then hide free? this forces behavior exactly

class DescriptorPageAllocator
{
public:

	DescriptorPageAllocator(
		ID3D12Device4* device,
		FenceView fence_view,
		D3D12_DESCRIPTOR_HEAP_DESC heap_desc
	);

	// modifies page heap and page indecies, can return null descriptor
	DescriptorAlloc create_descriptor_allocation(UINT requested_size);

	// modifies stale queue and nothing else, locks the stale queue
	void free_descriptor_allocation(UINT index, UINT offset, UINT size);

	// modifies both the stale queue and page pool and locks both but not at the same time
	void release_stale_descriptors();

protected:

	class DescriptorPage
	{
	public:

		DescriptorPage(
			ID3D12Device4* device,
			UINT size,
			D3D12_DESCRIPTOR_HEAP_TYPE type,
			D3D12_DESCRIPTOR_HEAP_FLAGS flags,
			UINT node_mask
		);

		bool create_descriptor_block(UINT requested_size, UINT& offset_out);

		void free_descriptor_block(UINT offset, UINT size);

		UINT count_of_free_descriptors() const noexcept {
			return mFreeDescCount;
		}

		D3D12_CPU_DESCRIPTOR_HANDLE handle(UINT index = 0u) const noexcept {
			return mHeap.handle(index);
		}

		UINT stride() const noexcept {
			return mHeap.stride();
		}

	protected:

		// modifies maps but does not lock
		void add_new_block(UINT offset, UINT size);

	private:

		struct FreeBlockInfo;
		using OffsetMap = std::map<UINT, FreeBlockInfo>;
		using SizeMap = std::multimap<UINT, OffsetMap::iterator>;

		struct FreeBlockInfo
		{
			FreeBlockInfo(UINT size_)
				: size(size_)
			{
			}

			UINT size;
			SizeMap::iterator size_map_iterator;
		};

		DescriptorHeap mHeap;
		UINT           mFreeDescCount;

		OffsetMap      mOffsetMap;
		SizeMap        mSizeMap;
	};

	using PDescPage = std::unique_ptr<DescriptorPage>;

	// modifies page pool and free pages list, but does not lock
	UINT64 create_page(UINT size);

private:

	struct StaleDescriptor
	{
		StaleDescriptor(UINT index_, UINT offset_, UINT size_, UINT64 fence_value_)
			:fence_value(fence_value_)
			, index(index_)
			, offset(offset_)
			, size(size_)
		{
		}
		UINT64 fence_value;
		UINT index;
		UINT offset;
		UINT size;
	};

	ViewPtr<ID3D12Device4>      mDevice;

	std::vector<PDescPage>      mPagePool;
	std::set<UINT64>            mFreePages;
	D3D12_DESCRIPTOR_HEAP_DESC  mHeapDesc;

	std::queue<StaleDescriptor> mStaleDescriptors;
	FenceView                   mFenceView;

	std::mutex                  mAllocLock;
	std::mutex                  mStaleLock;
};
