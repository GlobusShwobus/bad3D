#include "D3D12/DescriptorAlloc.h"

#include <stdexcept>

#include "D3D12/DescriptorPageAllocator.h"

DescriptorAlloc::~DescriptorAlloc() {
	free();
}

DescriptorAlloc::DescriptorAlloc(
	D3D12_CPU_DESCRIPTOR_HANDLE descriptor,
	UINT num_handles,
	UINT stride,
	DescriptorPageAllocator* allocator,
	UINT64 page_index
)
	:mStride(stride)
	, mSize(num_handles)
	, mBegin(descriptor)
	, mAllocator(allocator)
	, mPageIndex(page_index)
{
	if (!mAllocator)
		throw std::invalid_argument("Source nullptr");
}

DescriptorAlloc& DescriptorAlloc::operator=(DescriptorAlloc&& rhs) noexcept
{
	if (this != &rhs)
	{
		// Free this descriptor if it points to anything.
		free();
		mBegin = rhs.mBegin;
		mSize = rhs.mSize;
		mStride = rhs.mStride;
		mAllocator = rhs.mAllocator;
		mPageIndex = rhs.mPageIndex;

		rhs.mBegin.ptr = 0;
		rhs.mSize = 0;
		rhs.mStride = 0;
		rhs.mAllocator = nullptr;
		rhs.mPageIndex = 0;
	}
	return *this;
}

void DescriptorAlloc::free()
{
	if (!is_null() && mAllocator)
	{
		mAllocator->free_descriptor_alloc(mPageIndex, mBegin, mSize); // source must free this range. page being shared ptr is fine. maybe viewptr is fine not sure

		mBegin.ptr = 0;
		mSize = 0;
		mStride = 0;
		mAllocator = nullptr;
		mPageIndex = 0;
	}
}