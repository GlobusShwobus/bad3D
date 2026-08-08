#pragma once
#include "GPU_CORE.h"
#include "ObserverPtr.h"

class DX12DescriptorHeap
{
public:
	DX12DescriptorHeap(ObserverPtr<ID3D12Device4> device, UINT desc_count, D3D12_DESCRIPTOR_HEAP_TYPE type);
	virtual ~DX12DescriptorHeap() = default;

	DX12DescriptorHeap(const DX12DescriptorHeap&) = delete;
	DX12DescriptorHeap& operator=(const DX12DescriptorHeap&) = delete;
	DX12DescriptorHeap(DX12DescriptorHeap&&) = delete;
	DX12DescriptorHeap& operator=(DX12DescriptorHeap&&) = delete;

	constexpr UINT desc_size()const noexcept { return mDescriptorSize; }

	D3D12_CPU_DESCRIPTOR_HANDLE get_descriptor_handle_for(UINT index) const;

private:
	D3D12DescriptorHeap   mDescriptorHeap;
	UINT                  mDescriptorSize;
};