#include "D3D12/UploadBuffer.h"

#include <stdexcept>
#include <new>

#include "D3D12/EasyDirectX.h"
#include "D3D12/EasyDirectXUtils.h"
#include "Tools/Align.h"

UploadBuffer::UploadBuffer(ID3D12Device4* device, PageSize page_size)
	:mDevice(device), mPageSize(static_cast<UINT64>(page_size))
{
	if (!mDevice)
		throw std::invalid_argument("nullptr device");
}

UploadAddress UploadBuffer::allocate(UINT64 size_in_bytes, UINT64 alignment)
{
	// if requested size does not fit even a new page, then it's a bad request
	if (size_in_bytes > mPageSize)
		throw std::bad_alloc();

	// If there is no current page, or the requested allocation exceeds the
	// remaining space in the current page, request a new page.
	if (!mCurrentPage || !mCurrentPage->has_space(size_in_bytes, alignment))
		mCurrentPage = request_page();

	// bump offset
	return mCurrentPage->allocate(size_in_bytes, alignment);
}

std::shared_ptr<UploadBuffer::Page> UploadBuffer::request_page()
{
	std::shared_ptr<Page> page;

	// if there are available pages in the free page list, get one from there, otherwise create one and 
	// push it to the all pages pool

	if (!mFreePages.empty()) {
		page = mFreePages.front();
		mFreePages.pop_front();
	}
	else {
		page = std::make_shared<Page>(mDevice.get(), mPageSize);
		mPagePool.push_back(page);
	}

	return page;
}

void UploadBuffer::reset()
{
	mCurrentPage = nullptr;

	// reset all available pages (copy because mPagePool should retain its list of pages)
	mFreePages = mPagePool;

	for (auto& page : mFreePages)
		page->reset();
}

UploadBuffer::Page::Page(ID3D12Device4* device, UINT64 size_in_bytes)
	:mPageSize(size_in_bytes)
	, mPageOffset(0ULL)
	, mCPUPtr(nullptr)
	, mGPUPtr(D3D12_GPU_VIRTUAL_ADDRESS(0))
{
	mD3D12Resource = create_commited_resource(
		device,
		easy::heap_property_upload(),
		easy::resource_desc_buffer(size_in_bytes),
		D3D12_RESOURCE_STATE_GENERIC_READ
	);

	if (!mD3D12Resource)
		throw std::bad_alloc();

	mGPUPtr = mD3D12Resource->GetGPUVirtualAddress();
	mD3D12Resource->Map(0, nullptr, &mCPUPtr);
}

UploadBuffer::Page::~Page()
{
	mD3D12Resource->Unmap(0, nullptr);
	mCPUPtr = nullptr;
	mGPUPtr = D3D12_GPU_VIRTUAL_ADDRESS(0);
}

bool UploadBuffer::Page::has_space(UINT64 size_in_byes, UINT64 alignment) const noexcept
{
	// add alignment to obj
	const std::size_t aligned_size = align_up(size_in_byes, alignment);
	// bump the offset to the correct position (the start pos must also be aligned)
	const std::size_t aligned_offset = align_up(mPageOffset, alignment);

	return aligned_offset + aligned_size <= mPageSize;
}

UploadAddress UploadBuffer::Page::allocate(UINT64 size_in_byes, UINT64 alignment)
{
	//	if (!has_space(size_in_byes, alignment))
	//		throw std::bad_alloc();    // Can't allocate space from page.

	std::size_t aligned_size = align_up(size_in_byes, alignment);
	mPageOffset = align_up(mPageOffset, alignment);

	UploadAddress allocation;
	allocation.CPU = static_cast<unsigned char*>(mCPUPtr) + mPageOffset;
	allocation.GPU = mGPUPtr + mPageOffset;

	mPageOffset += aligned_size;

	return allocation;
}