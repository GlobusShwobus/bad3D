#pragma once

#include "badDirectX.h"
#include <wrl/client.h>
#include "ViewPtr.h"

class DescriptorHeap
{
public:
	DescriptorHeap(ViewPtr<ID3D12Device4> device, UINT desc_count, D3D12_DESCRIPTOR_HEAP_TYPE type, D3D12_DESCRIPTOR_HEAP_FLAGS flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE, UINT node_masks = 0);

	// dont wanna think about it atm
	DescriptorHeap(const DescriptorHeap&) = delete;
	DescriptorHeap& operator=(const DescriptorHeap&) = delete;
	DescriptorHeap(DescriptorHeap&&) = delete;
	DescriptorHeap& operator=(DescriptorHeap&&) = delete;

	D3D12_CPU_DESCRIPTOR_HANDLE get_descriptor_handle_for(SIZE_T index) const noexcept;
	D3D12_CPU_DESCRIPTOR_HANDLE get_desc_begin() const noexcept;

	constexpr UINT stride() const noexcept { return mStride; }
	constexpr UINT count() const noexcept { return mCount; }
	constexpr D3D12_DESCRIPTOR_HEAP_TYPE type() const noexcept { return mType; }

private:

	Microsoft::WRL::ComPtr<ID3D12DescriptorHeap>   mHeap = nullptr;
	UINT                                           mStride = 0;
	const UINT                                     mCount = 0;
	const D3D12_DESCRIPTOR_HEAP_TYPE               mType;
};