#pragma once

#include <d3d12.h>

#include "D3D12/GPUResource.h"
#include "Tools/ViewPtr.h"

struct GPUResourceAlloc
{
    GPUResourceAlloc()
        :buffer(nullptr)
        , offset(0ull)
        , size(0ull)
        , dataPtr(nullptr)
        , GPUAddress(D3D12_GPU_VIRTUAL_ADDRESS(NULL))
    {
    }

    GPUResourceAlloc(GPUResource* resource, UINT64 offset_, UINT64 size_, void* CPUptr, D3D12_GPU_VIRTUAL_ADDRESS GPUptr)
        :buffer(resource)
        , offset(offset_)
        , size(size_)
        , dataPtr(CPUptr)
        , GPUAddress(GPUptr)
    {
    }

    ViewPtr<GPUResource> buffer;            // non owning view for related resource
    UINT64 offset;                          // offset from start of the buffer
    UINT64 size;                            // size of this alloc

    void* dataPtr;                          // CPU mapped address
    D3D12_GPU_VIRTUAL_ADDRESS GPUAddress;   // GPU address
};