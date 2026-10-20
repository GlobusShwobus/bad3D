#pragma once

/**
*
* GPUBumpAllocator is heavily inspired by Jeremiah van Oosten https://www.3dgep.com/learning-directx-12-3/
* and Microsoft's DirectX mini engine https://github.com/microsoft/DirectX-Graphics-Samples/blob/master/MiniEngine/Core/LinearAllocator.h
* ...
*
* The classes in this header are not thread safe in of themselves.
*/

#include <d3d12.h>

#include "D3D12/GPUResource.h"
#include "Tools/ViewPtr.h"

struct DynGPUAlloc
{
    DynGPUAlloc()
        :buffer(nullptr), offset(0ull), size(0ull), dataPtr(nullptr), GPUAddress(D3D12_GPU_VIRTUAL_ADDRESS(NULL))
    {}

    DynGPUAlloc(GPUResource* resource, UINT64 offset_, UINT64 size_, void* CPUptr, D3D12_GPU_VIRTUAL_ADDRESS GPUptr)
        :buffer(resource), offset(offset_), size(size_), dataPtr(CPUptr), GPUAddress(GPUptr)
    {}

    ViewPtr<GPUResource> buffer;            // non owning view for related resource
    UINT64 offset;                          // offset from start of the buffer
    UINT64 size;                            // size of this alloc
                                    
    void* dataPtr;                          // CPU mapped address
    D3D12_GPU_VIRTUAL_ADDRESS GPUAddress;   // GPU address
};

class GPUBumpAllocator
{
public:
    GPUBumpAllocator() = delete;
    GPUBumpAllocator(const GPUBumpAllocator&) = delete;
    GPUBumpAllocator& operator=(const GPUBumpAllocator&) = delete;
    GPUBumpAllocator(GPUBumpAllocator&&) = delete;
    GPUBumpAllocator& operator=(GPUBumpAllocator&&) = delete;

    GPUBumpAllocator(GPUResource&& rresource, UINT64 size_in_bytes);

    // static std::shared_ptr<GPUPage> create_upload_page(ID3D12Device4* device, UINT64 size_in_bytes) {
    //     // can throw
    //     GPUResource resource(
    //         device,
    //         easy::heap_property_upload(),
    //         easy::resource_desc_buffer(size_in_bytes),
    //         D3D12_RESOURCE_STATE_GENERIC_READ
    //     );
    // 
    //     return std::shared_ptr<GPUPage>(new GPUPage{ std::move(resource), size_in_bytes });
    // }

    ~GPUBumpAllocator();

    bool has_space(UINT64 size_in_byes, UINT64 alignment) const noexcept;

    // Allocate memory from the page.
    // Throws std::bad_alloc if the the allocation size is larger
    // that the page size or the size of the allocation exceeds the 
    // remaining space in the page.
    DynGPUAlloc allocate(UINT64 size_in_bytes, UINT64 alignment);

    // reset the page for reuse
    void reset() noexcept {
        mOffset = 0;
    }

private:
    GPUResource mResource;

    //base ptr
    void* mCPUPtr;
    D3D12_GPU_VIRTUAL_ADDRESS mGPUPtr;

    // allocated page size
    UINT64 mSize;

    // current allocation offset
    UINT64 mOffset;
};