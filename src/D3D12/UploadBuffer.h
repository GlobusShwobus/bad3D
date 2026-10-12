#pragma once

#include <memory>
#include <deque>

#include <d3d12.h>
#include <wrl/client.h>

#include "Tools/ViewPtr.h"

constexpr unsigned long long KB(unsigned long long x) noexcept {
    return x * 1024ULL;
}

constexpr unsigned long long MB(unsigned long long x) noexcept {
    return x * 1024ULL * 1024ULL;
}

enum PageSize : unsigned long long
{
    _64KB = KB(64),
    _1MB = MB(1),
    _2MB = MB(2),
    _4MB = MB(4),
    _8MB = MB(8),
    _16MB = MB(16),
    _32MB = MB(32),
    _64MB = MB(64),
    _128MB = MB(128),
    _256MB = MB(256)
};

/**
* The source of this is https://www.3dgep.com/learning-directx-12-3/
* ...
* This class is not thread safe as it currently is intended to have 1 upload buffer class per 1 command list class.
* For a more global uploadbuffer class, threadsafety should be added in...
*/

struct UploadAddress
{
    void* CPU;
    D3D12_GPU_VIRTUAL_ADDRESS GPU;
};

class UploadBuffer
{
public:

    // page_size - the size to use to allocate new pages in the GPU memory
    explicit UploadBuffer(ID3D12Device4* device, PageSize page_size = _2MB);

    virtual ~UploadBuffer() = default;

    // the maximum size of an allocation is the size of a single page
    std::size_t get_page_size() const noexcept {
        return mPageSize;
    }

    // Allocate memory in an Upload heap.
    // An allocation must not exceed the size of a page.
    // Use a memcpy or similar method to copy the
    // buffer data to CPU pointer in the Allocation structure returned from
    // this function. 
    UploadAddress allocate(std::size_t size_in_bytes, std::size_t alignment);

    // release all allocated pages. this should only be done when the command list 
    // is finished executing on the command queue
    void reset();

private:

    class Page
    {
    public:
        Page(ID3D12Device4* device, UINT64 size_in_bytes);
        ~Page();
        bool has_space(UINT64 size_in_byes, UINT64 alignment) const noexcept;

        // Allocate memory from the page.
        // Throws std::bad_alloc if the the allocation size is larger
        // that the page size or the size of the allocation exceeds the 
        // remaining space in the page.
        UploadAddress allocate(UINT64 size_in_bytes, UINT64 alignment);

        // reset the page for reuse
        void reset() noexcept {
            mPageOffset = 0;
        }

    private:
        Microsoft::WRL::ComPtr<ID3D12Resource> mD3D12Resource;

        //base ptr
        void* mCPUPtr;
        D3D12_GPU_VIRTUAL_ADDRESS mGPUPtr;

        // allocated page size
        UINT64 mPageSize;

        // current allocation offset
        UINT64 mPageOffset;
    };

    // request a page from the pool of available pages or create a new page
    std::shared_ptr<Page> request_page();

    // a pool of memory pages
    std::deque<std::shared_ptr<Page>> mPagePool;
    std::deque<std::shared_ptr<Page>> mFreePages;

    std::shared_ptr<Page> mCurrentPage;

    ViewPtr<ID3D12Device4> mDevice;

    UINT64 mPageSize;
};