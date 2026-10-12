#include "D3D12/DescriptorRange.h"

#include <stdexcept>

#include "D3D12/DescriptorFreeListHeap.h"

DescriptorRange::~DescriptorRange() {
	free();
}

DescriptorRange::DescriptorRange(
	D3D12_CPU_DESCRIPTOR_HANDLE descriptor,
	UINT num_handles,
	UINT stride,
	DescriptorFreeListHeap* heap
)
	:mStride(stride)
	, mSize(num_handles)
	, mBegin(descriptor)
	, mHeap(heap)
{
	if (!mHeap)
		throw std::invalid_argument("Page nullptr");
}

DescriptorRange& DescriptorRange::operator=(DescriptorRange&& rhs) noexcept
{
	if (this != &rhs)
	{
		// Free this descriptor if it points to anything.
		free();
		mBegin = rhs.mBegin;
		mSize = rhs.mSize;
		mStride = rhs.mStride;
		mHeap = rhs.mHeap;

		rhs.mBegin.ptr = 0;
		rhs.mSize = 0;
		rhs.mStride = 0;
		rhs.mHeap = nullptr;
	}
	return *this;
}

void DescriptorRange::free()
{
	if (!is_null() && mHeap)
	{
		mHeap->free(mBegin, mSize); // page must free this range. page being shared ptr is fine. maybe viewptr is fine not sure

		mBegin.ptr = 0;
		mSize = 0;
		mStride = 0;
		mHeap = nullptr;
	}
}