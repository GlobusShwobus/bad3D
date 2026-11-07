#pragma once

#include <cstdint>
#include <memory>
#include <queue>
#include <functional>

#include <d3d12.h>
#include <wrl/client.h>

class CommandList;
class RootSignature;

class DynamicDesriptorHeap
{
public:
	DynamicDesriptorHeap(ID3D12Device4* device, D3D12_DESCRIPTOR_HEAP_TYPE type, UINT numDescriptorsPerHeap = 1024);

	virtual ~DynamicDesriptorHeap();


	// root_parameter_index - which root signature descriptor table recieves the descriptors
	// offset - offset inside the table of the root signature
	// num_descriptors - amount of descriptors to stage
	// begin - the first handle begin iteration
	void stage_descriptors(UINT root_parameter_index, UINT offset, UINT num_descriptors, const D3D12_CPU_DESCRIPTOR_HANDLE begin);


    /**
     * Copy all of the staged descriptors to the GPU visible descriptor heap and
     * bind the descriptor heap and the descriptor tables to the command list.
     * The passed-in function object is used to set the GPU visible descriptors
     * on the command list. Two possible functions are:
     *   * Before a draw    : ID3D12GraphicsCommandList::SetGraphicsRootDescriptorTable
     *   * Before a dispatch: ID3D12GraphicsCommandList::SetComputeRootDescriptorTable
     *
     * Since the DynamicDescriptorHeap can't know which function will be used, it must
     * be passed as an argument to the function.
     */

    void commit_staged_descriptors(CommandList& commandList, std::function<void(ID3D12GraphicsCommandList*, UINT, D3D12_GPU_DESCRIPTOR_HANDLE)> setfunc);
    void commit_staged_descriptors_draw(CommandList& commandList);
    void commit_staged_descriptors_dispatch(CommandList& commandList);


    /**
     * Copies a single CPU visible descriptor to a GPU visible descriptor heap.
     * This is useful for the
     *   * ID3D12GraphicsCommandList::ClearUnorderedAccessViewFloat
     *   * ID3D12GraphicsCommandList::ClearUnorderedAccessViewUint
     * methods which require both a CPU and GPU visible descriptors for a UAV
     * resource.
     *
     * @param commandList The command list is required in case the GPU visible
     * descriptor heap needs to be updated on the command list.
     * @param cpuDescriptor The CPU descriptor to copy into a GPU visible
     * descriptor heap.
     *
     * @return The GPU visible descriptor.
     */
    D3D12_GPU_DESCRIPTOR_HANDLE copy_descriptor(CommandList& commandList, D3D12_CPU_DESCRIPTOR_HANDLE cpu_descriptor);

    /**
     * Parse the root signature to determine which root parameters contain
     * descriptor tables and determine the number of descriptors needed for
     * each table.
     */

    void parse_root_signature(const RootSignature& rootsig);

    /**
     * Reset used descriptors. This should only be done if any descriptors
     * that are being referenced by a command list has finished executing on the
     * command queue.
     */
    void reset();

private:

    // Request a descriptor heap if one is available.
    Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> request_descriptor_heap();

    // Create a new descriptor heap of no descriptor heap is available.
    Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> create_descriptor_heap();

    // Compute the number of stale descriptors that need to be copied
    // to GPU visible descriptor heap.
    UINT compute_stale_descriptor_count() const;

    /**
     * The maximum number of descriptor tables per root signature.
     * A 32-bit mask is used to keep track of the root parameter indices that
     * are descriptor tables.
     */
    static constexpr UINT MaxDescriptorTables = 32;

    /**
     * A structure that represents a descriptor table entry in the root signature.
     */
    struct DescriptorTableCache
    {
        DescriptorTableCache()
            :num_descriptors(0), base_descriptor(nullptr)
        {
        }

        // Reset the table cache.
        void Reset()
        {
            num_descriptors = 0;
            base_descriptor = nullptr;
        }

        // The number of descriptors in this descriptor table.
        uint32_t num_descriptors;
        // The pointer to the descriptor in the descriptor handle cache.
        D3D12_CPU_DESCRIPTOR_HANDLE* base_descriptor;
    };

    // Describes the type of descriptors that can be staged using this 
    // dynamic descriptor heap.
    // Valid values are:
    //   * D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV
    //   * D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER
    // This parameter also determines the type of GPU visible descriptor heap to 
    // create.
    D3D12_DESCRIPTOR_HEAP_TYPE mDescriptorHeapType;

    // The number of descriptors to allocate in new GPU visible descriptor heaps.
    uint32_t mNumDescriptorsPerHeap;

    // The increment size of a descriptor.
    uint32_t mDescriptorStride;

    // The descriptor handle cache
    std::unique_ptr<D3D12_CPU_DESCRIPTOR_HANDLE[]> mDescriptorHandleCache;

    // Descriptor handle cache per descriptor table.
    DescriptorTableCache m_DescriptorTableCache[MaxDescriptorTables];

    // Each bit in the bit mask represents the index in the root signature
    // that contains a descriptor table.
    uint32_t mDescriptorTableBitMap;
    // Each bit set in the bit mask represents a descriptor table
    // in the root signature that has changed since the last time the 
    // descriptors were copied.
    uint32_t mStaleDescriptorTableBitMap;

    using DescriptorHeapPool = std::queue<Microsoft::WRL::ComPtr<ID3D12DescriptorHeap>>;

    // all descriptor heaps created
    DescriptorHeapPool mDescriptorHeapPool;
    // only those heaps which still contain descriptors
    DescriptorHeapPool mAvailableDescriptorHeaps;

    Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> mCurrentDescriptorHeap;
    D3D12_GPU_DESCRIPTOR_HANDLE mCurrentGPUDescriptorHandle;
    D3D12_CPU_DESCRIPTOR_HANDLE mCurrentCPUDescriptorHandle;

    uint32_t mNumFreeHandles;
};