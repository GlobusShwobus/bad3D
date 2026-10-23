#pragma once

/**
*
* GPUBumpAllocator is heavily inspired by Jeremiah van Oosten https://www.3dgep.com/learning-directx-12-3/
* and Microsoft's DirectX mini engine https://github.com/microsoft/DirectX-Graphics-Samples/blob/master/MiniEngine/Core/LinearAllocator.h
* ...
*
* GPUResourceBumpAllocator is not thread safe in of itself.
*/

#include <d3d12.h>

#include "D3D12/GPUResourceAlloc.h"
#include "Tools/ViewPtr.h"


class GPUResourceBumpAllocator
{
public:
    GPUResourceBumpAllocator() = delete;
    GPUResourceBumpAllocator(const GPUResourceBumpAllocator&) = delete;
    GPUResourceBumpAllocator& operator=(const GPUResourceBumpAllocator&) = delete;
    GPUResourceBumpAllocator(GPUResourceBumpAllocator&&) = delete;
    GPUResourceBumpAllocator& operator=(GPUResourceBumpAllocator&&) = delete;

    GPUResourceBumpAllocator(GPUResource&& rresource, UINT64 size_in_bytes);

    ~GPUResourceBumpAllocator();

    bool has_space(UINT64 size_in_byes, UINT64 alignment) const noexcept;

    // Allocate memory from the page.
    // Throws std::bad_alloc if the the allocation size is larger
    // that the page size or the size of the allocation exceeds the 
    // remaining space in the page.
    GPUResourceAlloc bump_allocate(UINT64 size_in_bytes, UINT64 alignment);

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