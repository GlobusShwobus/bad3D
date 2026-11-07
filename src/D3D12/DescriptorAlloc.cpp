#include "D3D12/DescriptorAlloc.h"

#include <stdexcept>

#include "D3D12/DescriptorPageAllocator.h"

DescriptorAlloc::~DescriptorAlloc() {
	free();
}

DescriptorAlloc& DescriptorAlloc::operator=(DescriptorAlloc&& rhs) noexcept
{
	if (this != &rhs)
	{
		// Free this descriptor if it points to anything.
		free();
		mHandle = rhs.mHandle;
		mSourceHeapOffset = rhs.mSourceHeapOffset;
		mSize = rhs.mSize;
		mStride = rhs.mStride;
		mAllocator = rhs.mAllocator;
		mPageIndex = rhs.mPageIndex;

		rhs.mHandle.ptr = 0;
		rhs.mAllocator = nullptr;
	}
	return *this;
}

void DescriptorAlloc::free()
{
	if (!is_null() && mAllocator)
	{
		mAllocator->free_descriptor_allocation(mPageIndex, mSourceHeapOffset, mSize);

		mHandle.ptr = 0;
		mAllocator = nullptr;
	}
}