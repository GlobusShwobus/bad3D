#include "D3D12/GPUResourceBumpAllocator.h"

#include <new>

#include "Tools/Align.h"

GPUBumpAllocator::~GPUBumpAllocator()
{
	if (mResource)// idk if this check is needed, technically no but it's prob a nothing burger anyway
		mResource->Unmap(0, nullptr);

	mCPUPtr = nullptr;
	mGPUPtr = D3D12_GPU_VIRTUAL_ADDRESS(NULL);
}

bool GPUBumpAllocator::has_space(UINT64 size_in_byes, UINT64 alignment) const noexcept
{
	// add alignment to obj
	const std::size_t aligned_size = align_up(size_in_byes, alignment);
	// bump the offset to the correct position (the start pos must also be aligned)
	const std::size_t aligned_offset = align_up(mOffset, alignment);

	return aligned_offset + aligned_size <= mSize;
}

DynGPUAlloc GPUBumpAllocator::allocate(UINT64 size_in_bytes, UINT64 alignment)
{
	if (!has_space(size_in_bytes, alignment))
		throw std::bad_alloc();    // Can't allocate space from page.

	// first align the size and offset positions
	std::size_t aligned_size = align_up(size_in_bytes, alignment);
	mOffset = align_up(mOffset, alignment);

	// create allocation
	DynGPUAlloc allocation(
		&mResource,
		mOffset,
		size_in_bytes,
		static_cast<uint8_t*>(mCPUPtr) + mOffset,
		mGPUPtr + mOffset
	);

	// bump offset 
	mOffset += aligned_size;

	return allocation;
}

GPUBumpAllocator::GPUBumpAllocator(GPUResource&& rresource, UINT64 size_in_bytes)
	:mResource(std::move(rresource))
	, mCPUPtr(nullptr)
	, mGPUPtr(D3D12_GPU_VIRTUAL_ADDRESS(NULL))
	, mSize(size_in_bytes)
	, mOffset(0ull)
{
	// GPUResource by this point must be valid
	mGPUPtr = mResource.address();
	mResource->Map(0, nullptr, &mCPUPtr);
}
