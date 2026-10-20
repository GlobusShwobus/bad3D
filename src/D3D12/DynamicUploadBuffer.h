#pragma once

#include <memory>
#include <deque>

#include <d3d12.h>
#include "Tools/ViewPtr.h"
#include "Tools/PageSize.h"
#include "D3D12/GPUResourceBumpAllocator.h"

class DynamicUploadBuffer
{
public:

    // page_size - the size to use to allocate new pages in the GPU memory
    explicit DynamicUploadBuffer(ID3D12Device4* device, PageSize page_size = _2MB);

    virtual ~DynamicUploadBuffer() = default;

    // the maximum size of an allocation is the size of a single page
    std::size_t get_page_size() const noexcept {
        return mPageSize;
    }

    // Allocate memory in an Upload heap.
    // An allocation must not exceed the size of a page.
    // Use a memcpy or similar method to copy the
    // buffer data to CPU pointer in the Allocation structure returned from
    // this function. 
    DynGPUAlloc allocate(std::size_t size_in_bytes, std::size_t alignment);

    // release all allocated pages. this should only be done when the command list 
    // is finished executing on the command queue
    void reset();

protected:

    // request a page from the pool of available pages or create a new page
    ViewPtr<GPUBumpAllocator> request_page();

private:

    using PageDeque = std::deque<std::shared_ptr<GPUBumpAllocator>>;

    // a pool of memory pages
    PageDeque mAllPages;
    PageDeque mFreePages;

    ViewPtr<GPUBumpAllocator> mCurrentPage;

    ViewPtr<ID3D12Device4> mDevice;

    UINT64 mPageSize;
};