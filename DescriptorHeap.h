#pragma once

#include "badDirectX.h"
#include <wrl/client.h>

class DescriptorHeap final
{
public:
	DescriptorHeap() = default;
	DescriptorHeap(ID3D12Device4* device, UINT desc_count, D3D12_DESCRIPTOR_HEAP_TYPE type, D3D12_DESCRIPTOR_HEAP_FLAGS flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE, UINT node_masks = 0);
	~DescriptorHeap() = default;
	// dont wanna think about it atm
	DescriptorHeap(const DescriptorHeap&) = delete;
	DescriptorHeap& operator=(const DescriptorHeap&) = delete;


	DescriptorHeap(DescriptorHeap&&) noexcept;
	DescriptorHeap& operator=(DescriptorHeap&&) noexcept;

	D3D12_CPU_DESCRIPTOR_HANDLE descriptor_at(SIZE_T index) const noexcept;
	constexpr D3D12_CPU_DESCRIPTOR_HANDLE descriptor_begin() const noexcept { return mBegin; }

	constexpr UINT stride() const noexcept { return mStride; }
	constexpr UINT count() const noexcept  { return mCount; }
	constexpr D3D12_DESCRIPTOR_HEAP_TYPE type() const noexcept { return mType; }

private:

	Microsoft::WRL::ComPtr<ID3D12DescriptorHeap>   mHeap = nullptr;
	UINT                                           mStride = 0;
	UINT                                           mCount = 0;
	D3D12_DESCRIPTOR_HEAP_TYPE                     mType{};

	D3D12_CPU_DESCRIPTOR_HANDLE                    mBegin{};
};