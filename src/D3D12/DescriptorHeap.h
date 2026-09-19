#pragma once

#include <d3d12.h>
#include <wrl/client.h>

class DescriptorHeap final
{
public:
	DescriptorHeap() = default;
	DescriptorHeap(ID3D12Device4* device, const D3D12_DESCRIPTOR_HEAP_DESC& desc);

	// dont wanna think about it atm
	DescriptorHeap(const DescriptorHeap&) = delete;
	DescriptorHeap& operator=(const DescriptorHeap&) = delete;
	DescriptorHeap(DescriptorHeap&&) noexcept = default;
	DescriptorHeap& operator=(DescriptorHeap&&) noexcept = default;

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