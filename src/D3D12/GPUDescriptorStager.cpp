#include <D3D12/GPUDescriptorStager.h>

#include <assert.h>
#include <stdexcept>

#include "D3D12/CommandList.h"
#include "D3D12/RootSignature.h"

#include "D3D12/EasyDirectXUtils.h"


GPUDescriptorStager::GPUDescriptorStager(
	ID3D12Device4* device, 
	D3D12_DESCRIPTOR_HEAP_TYPE type, 
	UINT numDescriptorsPerHeap)
	:mDevice(device)
	, mDescriptorHeapType(type)
	, mNumDescriptorsPerHeap(numDescriptorsPerHeap)
	, mDescriptorTableBitMap(0)
	, mStaleDescriptorTableBitMap(0)
	, mCurrentCPUDescriptorHandle{0}
	, mCurrentGPUDescriptorHandle{0}
	, mNumFreeHandles(0)
{
	mDescriptorStride = mDevice->GetDescriptorHandleIncrementSize(mDescriptorHeapType);

	mDescriptorHandleCache = std::make_unique<D3D12_CPU_DESCRIPTOR_HANDLE[]>(mNumDescriptorsPerHeap);
}

void GPUDescriptorStager::stage_descriptors(UINT root_parameter_index, UINT offset, UINT num_descriptors, const D3D12_CPU_DESCRIPTOR_HANDLE begin)
{
	// Cannot stage more than the maximum number of descriptors per heap. (meaning there can be multiple GPU visible heaps at the end but a single stage can't go over max)
	// Cannot stage more than MaxDescriptorTables root parameters.
	if (num_descriptors > mNumDescriptorsPerHeap || root_parameter_index >= MaxDescriptorTables)
	{
		throw std::bad_alloc{};
	}

	DescriptorTableCache& descriptorTableCache = mDescriptorTableCache[root_parameter_index];

	// Check that the number of descriptors to copy does not exceed the number
	// of descriptors expected in the descriptor table.
	if ((offset + num_descriptors) > descriptorTableCache.num_descriptors)
	{
		std::length_error{ "number of descriptors exeeds the number expected by the tabelerino" };
	}

	D3D12_CPU_DESCRIPTOR_HANDLE* dstDescriptor = (descriptorTableCache.base_descriptor + offset);
	for (UINT i = 0; i < num_descriptors; ++i)
	{
		dstDescriptor[i] = D3D12_CPU_DESCRIPTOR_HANDLE{ dstDescriptor->ptr + (i + mDescriptorStride) };
	}

	// Set the root parameter index bit to make sure the descriptor table 
	// at that index is bound to the command list.
	mStaleDescriptorTableBitMap |= (1 << root_parameter_index);
}

void GPUDescriptorStager::parse_root_signature(const RootSignature& rootsig)
{
	//if the root signature changes, all descriptors must be rebound to the command list
	mStaleDescriptorTableBitMap = 0;

	//const auto& rootSignatureDesc = rootSignature.GetRootSignatureDesc();

	// Get a bit mask that represents the root parameter indices that match the 
	// descriptor heap type for this dynamic descriptor heap.
	mDescriptorTableBitMap = rootsig.CBV_SRV_UAV_table_bitmap();
	uint32_t descriptorTableBitMask = mDescriptorTableBitMap;

	uint32_t currentOffset = 0;
	DWORD root_index;

	// translate bit to index and also make sure the index does not go over the array
	while (_BitScanForward(&root_index, descriptorTableBitMask) && root_index < rootsig.parameter_count())
	{
		const UINT num_descriptors = rootsig.num_descriptors_for_table(root_index);

		DescriptorTableCache& descriptorTableCahce = mDescriptorTableCache[root_index];
		descriptorTableCahce.num_descriptors = num_descriptors;
		descriptorTableCahce.base_descriptor = mDescriptorHandleCache.get() + currentOffset;
		// bump it up
		currentOffset += num_descriptors;

		// Flip the descriptor table bit so it's not scanned again for the current index.
		descriptorTableBitMask ^= (1 << root_index);
	}

	// Make sure the maximum number of descriptors per descriptor heap has not been exceeded.
	assert(currentOffset <= mNumDescriptorsPerHeap && 
		"The root signature requires more than the maximum number of descriptors per descriptor heap. Consider increasing the maximum number of descriptors per descriptor heap.");
}

Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> GPUDescriptorStager::request_descriptor_heap()
{
	Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> descriptorHeap;

	if (!mAvailableDescriptorHeaps.empty())
	{
		descriptorHeap = mAvailableDescriptorHeaps.front();
		mAvailableDescriptorHeaps.pop();
	}
	else {
		descriptorHeap = create_descriptor_heap();
		mDescriptorHeapPool.push(descriptorHeap);
	}

	return descriptorHeap;
}

Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> GPUDescriptorStager::create_descriptor_heap()
{
	D3D12_DESCRIPTOR_HEAP_DESC heap_desc{};
	heap_desc.Type = mDescriptorHeapType;
	heap_desc.NumDescriptors = mNumDescriptorsPerHeap;
	heap_desc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;

	Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> descriptorHeap;

	execute_and_test_hresult(
		mDevice->CreateDescriptorHeap(&heap_desc, IID_PPV_ARGS(&descriptorHeap))
	);

	return descriptorHeap;
}

UINT GPUDescriptorStager::compute_stale_descriptor_count() const
{
	UINT num_stale_descriptors = 0;
	DWORD i;
	DWORD stale_descriptor_bitmap = mStaleDescriptorTableBitMap;

	while (_BitScanForward(&i, stale_descriptor_bitmap))
	{
		num_stale_descriptors += mDescriptorTableCache[i].num_descriptors;
		stale_descriptor_bitmap ^= (1 << i);
	}

	return num_stale_descriptors;
}
