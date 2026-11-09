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
		throw std::length_error{ "number of descriptors exeeds the number expected by the tabelerino" };
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

void GPUDescriptorStager::commit_staged_descriptors(CommandList& commandList, std::function<void(ID3D12GraphicsCommandList*, UINT, D3D12_GPU_DESCRIPTOR_HANDLE)> setfunc)
{
	// Compute the number of descriptors that need to be copied 
	uint32_t numDescriptorsToCommit = compute_stale_descriptor_count();

	// if nothing to do
	if (numDescriptorsToCommit == 0)
		return;

	auto* d3d12GraphicsCommandList = commandList.get_graphics_command_list();
	assert(d3d12GraphicsCommandList != nullptr);


	// if the current heap is null or not enough space in it, request a new one
	if (!mCurrentDescriptorHeap || mNumFreeHandles < numDescriptorsToCommit)
	{
		mCurrentDescriptorHeap = request_descriptor_heap();
		mCurrentCPUDescriptorHandle = mCurrentDescriptorHeap->GetCPUDescriptorHandleForHeapStart();
		mCurrentGPUDescriptorHandle = mCurrentDescriptorHeap->GetGPUDescriptorHandleForHeapStart();
		mNumFreeHandles = mNumDescriptorsPerHeap;

		commandList.set_descriptor_heap(mDescriptorHeapType, mCurrentDescriptorHeap.Get());

		// When updating the descriptor heap on the command list, all descriptor
		// tables must be (re)recopied to the new descriptor heap (not just
		// the stale descriptor tables).
		mStaleDescriptorTableBitMap = mDescriptorTableBitMap;
	}

	// (re)set tables marked as stale
	DWORD root_index;
	while (_BitScanForward(&root_index, mStaleDescriptorTableBitMap))
	{
		UINT num_src_descriptors = mDescriptorTableCache[root_index].num_descriptors;
		D3D12_CPU_DESCRIPTOR_HANDLE* pSrcDescriptorhandle = mDescriptorTableCache[root_index].base_descriptor;
	
		D3D12_CPU_DESCRIPTOR_HANDLE pDestDescriptorRnageStarts[] = { mCurrentCPUDescriptorHandle };
		UINT pDestDescriptorRangeSizes[] = { num_src_descriptors };
	
		// todo: be a clever ass and copy once?
		mDevice->CopyDescriptors(
			1,
			pDestDescriptorRnageStarts, 
			pDestDescriptorRangeSizes,
			num_src_descriptors,
			pSrcDescriptorhandle,
			nullptr, 
			mDescriptorHeapType
		);

		// Set the descriptors on the command list using the passed-in setter function.
		setfunc(d3d12GraphicsCommandList, root_index, mCurrentGPUDescriptorHandle);

		mCurrentCPUDescriptorHandle.ptr = mCurrentCPUDescriptorHandle.ptr + (num_src_descriptors * mDescriptorStride);
		mCurrentGPUDescriptorHandle.ptr = mCurrentGPUDescriptorHandle.ptr + (num_src_descriptors * mDescriptorStride);
		mNumFreeHandles -= num_src_descriptors;

		// Flip the stale bit so the descriptor table is not recopied again unless it is updated with a new descriptor.
		mStaleDescriptorTableBitMap ^= (1 << root_index);
	}
}

void GPUDescriptorStager::commit_staged_descriptors_draw(CommandList& commandList)
{
	commit_staged_descriptors(commandList, &ID3D12GraphicsCommandList::SetGraphicsRootDescriptorTable);
}

void GPUDescriptorStager::commit_staged_descriptors_dispatch(CommandList& commandList)
{
	commit_staged_descriptors(commandList, &ID3D12GraphicsCommandList::SetComputeRootDescriptorTable);
}

D3D12_GPU_DESCRIPTOR_HANDLE GPUDescriptorStager::copy_descriptor(CommandList& commandList, D3D12_CPU_DESCRIPTOR_HANDLE cpu_descriptor)
{
	if (!mCurrentDescriptorHeap || mNumFreeHandles < 1)
	{
		mCurrentDescriptorHeap = request_descriptor_heap();
		mCurrentCPUDescriptorHandle = mCurrentDescriptorHeap->GetCPUDescriptorHandleForHeapStart();
		mCurrentGPUDescriptorHandle = mCurrentDescriptorHeap->GetGPUDescriptorHandleForHeapStart();
		mNumFreeHandles = mNumDescriptorsPerHeap;

		commandList.set_descriptor_heap(mDescriptorHeapType, mCurrentDescriptorHeap.Get());

		// When updating the descriptor heap on the command list, all descriptor
		// tables must be (re)recopied to the new descriptor heap (not just
		// the stale descriptor tables).
		mStaleDescriptorTableBitMap = mDescriptorTableBitMap;
	}

	D3D12_GPU_DESCRIPTOR_HANDLE hGPU = mCurrentGPUDescriptorHandle;
	mDevice->CopyDescriptorsSimple(
		1,
		mCurrentCPUDescriptorHandle, 
		cpu_descriptor,
		mDescriptorHeapType
	);

	mCurrentCPUDescriptorHandle.ptr = mCurrentCPUDescriptorHandle.ptr + (1 * mDescriptorStride);
	mCurrentGPUDescriptorHandle.ptr = mCurrentGPUDescriptorHandle.ptr + (1 * mDescriptorStride);
	mNumFreeHandles -= 1;

	return hGPU;
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

void GPUDescriptorStager::reset()
{
	mAvailableDescriptorHeaps = mDescriptorHeapPool;
	mCurrentDescriptorHeap.Reset();
	mCurrentCPUDescriptorHandle = { 0 };
	mCurrentGPUDescriptorHandle = { 0 };
	mNumFreeHandles = 0;
	mDescriptorTableBitMap = 0;
	mStaleDescriptorTableBitMap = 0;

	// reset the table cache
	for (int i = 0; i < MaxDescriptorTables; i++)
	{
		mDescriptorTableCache[i].Reset();
	}
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
