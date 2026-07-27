#include "DX12DescriptorHeap.h"
#include "Utils.h"

void DX12DescriptorHeap::initialise(ObserverPtr<ID3D12Device2> device, D3D12_DESCRIPTOR_HEAP_DESC desc)
{
	execute_test_throw(
		device->CreateDescriptorHeap(&desc, IID_PPV_ARGS(&mDescriptorHeap))
	);

	mDescriptorSize = device->GetDescriptorHandleIncrementSize(desc.Type);
}

void DX12DescriptorHeap::reset()
{
	mDescriptorHeap.Reset();
	mDescriptorSize = 0;
}

D3D12_CPU_DESCRIPTOR_HANDLE DX12DescriptorHeap::get_descriptor_handle_for(UINT index) const
{
	D3D12_CPU_DESCRIPTOR_HANDLE handle = { 0 };
	handle.ptr += mDescriptorHeap->GetCPUDescriptorHandleForHeapStart().ptr + (index * mDescriptorSize);
	return handle;
}