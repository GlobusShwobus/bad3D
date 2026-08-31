#include "DescriptorHeap.h"
#include "Utils.h"
#include "EasyDirectX.h"

DescriptorHeap::DescriptorHeap(ViewPtr<ID3D12Device4> device, UINT desc_count, D3D12_DESCRIPTOR_HEAP_TYPE type, D3D12_DESCRIPTOR_HEAP_FLAGS flags, UINT node_masks)
	:mType(type)
{
	assert(device && "device nullptr");
	
	D3D12_DESCRIPTOR_HEAP_DESC desc = DESC_HEAP::custom(desc_count, type, flags, node_masks);

	execute_and_test_hresult(
		device->CreateDescriptorHeap(&desc, IID_PPV_ARGS(&mHeap))
	);

	mStride = device->GetDescriptorHandleIncrementSize(type);
}

D3D12_CPU_DESCRIPTOR_HANDLE DescriptorHeap::get_descriptor_handle_for(SIZE_T index) const noexcept
{
	D3D12_CPU_DESCRIPTOR_HANDLE handle = { 0 };
	// pointer arithmetic, offset from begin to index times size in bytes
	handle.ptr = mHeap->GetCPUDescriptorHandleForHeapStart().ptr
		+ index * mStride;
	return handle;
}

D3D12_CPU_DESCRIPTOR_HANDLE DescriptorHeap::get_desc_begin() const noexcept
{
	return get_descriptor_handle_for(0);
}
