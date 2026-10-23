#include "D3D12/DescriptorAlloc.h"

#include <stdexcept>

#include "D3D12/DescriptorPage.h"

DescriptorAlloc::~DescriptorAlloc() {
	free();
}

DescriptorAlloc::DescriptorAlloc(
	D3D12_CPU_DESCRIPTOR_HANDLE descriptor,
	UINT num_handles,
	UINT stride,
	DescriptorPage* source
)
	:mStride(stride)
	, mSize(num_handles)
	, mBegin(descriptor)
	, mSource(source)
{
	if (!mSource)
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
		mSource = rhs.mSource;

		rhs.mBegin.ptr = 0;
		rhs.mSize = 0;
		rhs.mStride = 0;
		rhs.mSource = nullptr;
	}
	return *this;
}

void DescriptorAlloc::free()
{
	if (!is_null() && mSource)
	{
		mSource->free(mBegin, mSize); // source must free this range. page being shared ptr is fine. maybe viewptr is fine not sure

		mBegin.ptr = 0;
		mSize = 0;
		mStride = 0;
		mSource = nullptr;
	}
}