#include "DX12DescriptorHeap.h"
#include "Utils.h"

DX12DescriptorHeap::DX12DescriptorHeap(ObserverPtr<ID3D12Device4> device, UINT desc_count, D3D12_DESCRIPTOR_HEAP_TYPE type)
{
	D3D12_DESCRIPTOR_HEAP_DESC descriptor_heap_desc = {};
	descriptor_heap_desc.NumDescriptors = desc_count;
	descriptor_heap_desc.Type = type;
	descriptor_heap_desc.NodeMask = 0;
	descriptor_heap_desc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;

	execute_and_test_hresult(
		device->CreateDescriptorHeap(&descriptor_heap_desc, IID_PPV_ARGS(&mDescriptorHeap))
	);

	mDescriptorSize = device->GetDescriptorHandleIncrementSize(type);
}


D3D12_CPU_DESCRIPTOR_HANDLE DX12DescriptorHeap::get_descriptor_handle_for(UINT index) const
{
	D3D12_CPU_DESCRIPTOR_HANDLE handle = { 0 };
	// pointer arithmetic, offset from begin to index times size in bytes
	handle.ptr = mDescriptorHeap->GetCPUDescriptorHandleForHeapStart().ptr
		+ static_cast<SIZE_T>(index) * mDescriptorSize;
	return handle;
}