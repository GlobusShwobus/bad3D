#pragma once

#include <d3d12.h>

#include "D3D12/GPUResource.h"
#include "Tools/ViewPtr.h"

class GPUResourceAlloc
{
public:

    GPUResourceAlloc()
        :mResource(nullptr)
        , mOffset(0ull)
        , mSize(0ull)
        , mCPUAddress(nullptr)
        , mGPUAddress(D3D12_GPU_VIRTUAL_ADDRESS(NULL))
    {
    }

protected:

    friend class GPUResourceBumpAllocator;

    GPUResourceAlloc(GPUResource* resource, UINT64 offset_, UINT64 size_, ViewPtr<void> CPUptr, D3D12_GPU_VIRTUAL_ADDRESS GPUptr)
        :mResource(resource)
        , mOffset(offset_)
        , mSize(size_)
        , mCPUAddress(CPUptr)
        , mGPUAddress(GPUptr)
    {
    }

    GPUResource* resource() const { return mResource.get(); }
    UINT64 offset()const { return mOffset; }
    UINT64 size() const { return mSize; }

    void* cpu_address()const { return mCPUAddress.get(); }
    D3D12_GPU_VIRTUAL_ADDRESS gpu_address() const { return mGPUAddress; }

private:

    ViewPtr<GPUResource> mResource;          // non owning view for related resource
    UINT64 mOffset;                          // offset from start of the buffer
    UINT64 mSize;                            // size of this alloc

    ViewPtr<void> mCPUAddress;               // CPU mapped address
    D3D12_GPU_VIRTUAL_ADDRESS mGPUAddress;   // GPU address
};