#pragma once

#include <cstdint>
#include <memory>
#include <queue>
#include <functional>

#include <d3d12.h>
#include <wrl/client.h>

#include "Tools/ViewPtr.h"
#include "D3D12/Globals.h"

class CommandList;
class RootSignature;

/*
* The class first parses a root signature and creates a mimic layout of the root sig inside itself.
* 
* The class contains two main structures: a simple scratch array of handles and the root signature mimic where
* each root parameter (if it is a table...) will map into the scratch array caching size + location.
* 
* When CPU handles are staged, the root signature mimic is used to know where to stage into, not the scratch table itself
* but since they are mapped via pointers, it is important to keep in mind when writing to the root signature table, it means
* writing into the scratch array.
* 
* When all descriptors are staged, it is time to commit them and write (copy) them to a shader visible descriptor heap.
* 
*/

class GPUDescriptorStager
{
public:

    GPUDescriptorStager(ID3D12Device4* device, D3D12_DESCRIPTOR_HEAP_TYPE type, UINT numDescriptorsPerHeap = 1024);

	void stage_descriptors(UINT root_parameter_index, UINT offset, UINT num_descriptors, const D3D12_CPU_DESCRIPTOR_HANDLE begin);

    void commit_staged_descriptors(CommandList& commandList, std::function<void(ID3D12GraphicsCommandList*, UINT, D3D12_GPU_DESCRIPTOR_HANDLE)> setfunc);
    void commit_staged_descriptors_draw(CommandList& commandList);
    void commit_staged_descriptors_dispatch(CommandList& commandList);

    D3D12_GPU_DESCRIPTOR_HANDLE copy_descriptor(CommandList& commandList, D3D12_CPU_DESCRIPTOR_HANDLE cpu_descriptor);

    void parse_root_signature(const RootSignature& rootsig);

    // reset used descriptors. This should only be done only when the command queue is not referencing any descs.
    void reset();

private:

    Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> request_descriptor_heap();
    Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> create_descriptor_heap();

    // Compute the number of stale descriptors that need to be copied
    // to GPU visible descriptor heap.
    UINT compute_stale_descriptor_count() const;

    //A structure that represents a descriptor table entry in the root signature.
    struct DescriptorTableEntry
    {
        DescriptorTableEntry()
            :size(0), base_descriptor(nullptr)
        {
        }

        void Reset() {
            size = 0;
            base_descriptor = nullptr;
        }

        uint32_t size;
        D3D12_CPU_DESCRIPTOR_HANDLE* base_descriptor;
    };

    ViewPtr<ID3D12Device4> mDevice;

    D3D12_DESCRIPTOR_HEAP_TYPE mDescriptorHeapType;
    uint32_t mNumDescriptorsPerHeap;
    uint32_t mDescriptorStride;

    // staged CPU side descriptor handles
    std::unique_ptr<D3D12_CPU_DESCRIPTOR_HANDLE[]> mHandleCache;

    // root signature representation
    DescriptorTableEntry mTableCache[Globals::ROOT_PARAMETER_LIMIT];
    uint32_t mDescriptorTableBitMap;

    // bitmap of descriptors that need to be updated
    uint32_t mStaleDescriptorTableBitMap;

    using DescriptorHeapPool = std::queue<Microsoft::WRL::ComPtr<ID3D12DescriptorHeap>>;

    // all descriptor heaps created
    DescriptorHeapPool mHeapPool;
    // heaps available for reuse
    DescriptorHeapPool mAvailableHeaps;

    Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> mCurrentDescriptorHeap;
    D3D12_GPU_DESCRIPTOR_HANDLE mCurrentGPUDescriptorHandle;
    D3D12_CPU_DESCRIPTOR_HANDLE mCurrentCPUDescriptorHandle;

    uint32_t mNumFreeHandles;
};