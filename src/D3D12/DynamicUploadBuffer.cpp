#include "D3D12/DynamicUploadBuffer.h"

#include <stdexcept>

#include "D3D12/EasyDirectX.h"

DynamicUploadBuffer::DynamicUploadBuffer(ID3D12Device4* device, PageSize page_size)
	:mDevice(device), mPageSize(static_cast<UINT64>(page_size))
{
	if (!mDevice)
		throw std::invalid_argument("nullptr device");
}

ViewPtr<GPUResourceBumpAllocator> DynamicUploadBuffer::request_page()
{
	std::shared_ptr<GPUResourceBumpAllocator> page;

	// if there are available pages in the free page list, get one from there, otherwise create one and 
	// push it to the all pages pool

	if (!mFreePages.empty()) {
		page = mFreePages.front();
		mFreePages.pop_front();
	}
	else {
		page = std::make_shared<GPUResourceBumpAllocator>(
			GPUResource{ mDevice.get(),
				easy::heap_property_upload(),
				easy::resource_desc_buffer(mPageSize),
				D3D12_RESOURCE_STATE_GENERIC_READ 
			},
			mPageSize
		);

		mAllPages.push_back(page);
	}

	return ViewPtr{ page.get() };
}

GPUResourceAlloc DynamicUploadBuffer::bump_allocate(std::size_t size_in_bytes, std::size_t alignment)
{
	// if requested size does not fit even a new page, then it's a bad request
	if (size_in_bytes > mPageSize)
		throw std::bad_alloc();

	// If there is no current page, or the requested allocation exceeds the
	// remaining space in the current page, request a new page.
	if (!mCurrentPage || !mCurrentPage->has_space(size_in_bytes, alignment))
		mCurrentPage = request_page();

	// bump offset
	return mCurrentPage->bump_allocate(size_in_bytes, alignment);
}

void DynamicUploadBuffer::reset()
{
	mCurrentPage = nullptr;

	// reset all available pages (copy because mPagePool should retain its list of pages)
	mFreePages = mAllPages;

	for (auto& page : mFreePages)
		page->reset();
}