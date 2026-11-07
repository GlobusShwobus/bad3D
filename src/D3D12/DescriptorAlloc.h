#pragma once

#include <d3d12.h>

#include "Tools/ViewPtr.h"

class DescriptorAlloc
{
public:

	constexpr DescriptorAlloc() noexcept
		:mHandle{}
		, mSourceHeapOffset(0)
		, mSize(0)
		, mStride(0)
		, mAllocator(nullptr)
		, mPageIndex(0)
	{
	}

	~DescriptorAlloc();

	DescriptorAlloc(const DescriptorAlloc&) = delete;
	DescriptorAlloc& operator=(const DescriptorAlloc&) = delete;

	DescriptorAlloc(DescriptorAlloc&& rhs) noexcept
		: mHandle(rhs.mHandle)
		, mSourceHeapOffset(rhs.mSourceHeapOffset)
		, mSize(rhs.mSize)
		, mStride(rhs.mStride)
		, mAllocator(rhs.mAllocator)
		, mPageIndex(rhs.mPageIndex)
	{
		rhs.mAllocator = nullptr;
		rhs.mHandle.ptr = 0;
	}

	DescriptorAlloc& operator=(DescriptorAlloc&& rhs) noexcept;

	bool is_null() const noexcept {
		return mHandle.ptr == 0;
	}

	// Get a descriptor at a particular offset in the allocation.
	D3D12_CPU_DESCRIPTOR_HANDLE handle(UINT index = 0) const noexcept {
		assert(index < mSize);
		return { mHandle.ptr + (mStride * index) };
	}

	// Get the number of (consecutive) handles for this allocation.
	UINT size() const noexcept {
		return mSize;
	}

private:

	friend class DescriptorPageAllocator;

	DescriptorAlloc(
		D3D12_CPU_DESCRIPTOR_HANDLE handle,
		UINT source_heap_offset,
		UINT size,
		UINT stride,
		DescriptorPageAllocator* allocator,
		UINT page_index
		)
		:mHandle(handle)
		, mSourceHeapOffset(source_heap_offset)
		, mSize(size)
		, mStride(stride)
		, mAllocator(allocator)
		, mPageIndex(page_index)
	{}

	void free();


	ViewPtr<DescriptorPageAllocator> mAllocator;
	D3D12_CPU_DESCRIPTOR_HANDLE mHandle;

	UINT mPageIndex;
	UINT mSourceHeapOffset;
	UINT mSize;
	UINT mStride;
};