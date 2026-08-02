#include "DX12DescriptorHeap.h"
#include "Utils.h"

DX12DescriptorHeap::DX12DescriptorHeap(D3D12_DESCRIPTOR_HEAP_DESC desc, ObserverPtr<ID3D12Device4> device)
{
	execute_and_test_hresult(
		device->CreateDescriptorHeap(&desc, IID_PPV_ARGS(&mDescriptorHeap))
	);

	mDescriptorSize = device->GetDescriptorHandleIncrementSize(desc.Type);
}


D3D12_CPU_DESCRIPTOR_HANDLE DX12DescriptorHeap::get_descriptor_handle_for(UINT index) const
{
	D3D12_CPU_DESCRIPTOR_HANDLE handle = { 0 };
	// pointer arithmetic, offset from begin to index times size in bytes
	handle.ptr = mDescriptorHeap->GetCPUDescriptorHandleForHeapStart().ptr
		+ static_cast<SIZE_T>(index) * mDescriptorSize;
	return handle;
}