#pragma once

#include <stdexcept>

#include <d3d12.h>
#include <wrl/client.h>

class DescriptorHeap final
{
public:
	DescriptorHeap() = delete;

	DescriptorHeap(ID3D12Device4* device, const D3D12_DESCRIPTOR_HEAP_DESC& desc)
	{
		if (!device)
			throw std::invalid_argument{ "nullptr device" };

		execute_and_test_hresult(
			device->CreateDescriptorHeap(&desc, IID_PPV_ARGS(&mHeap))
		);

		mBegin = mHeap->GetCPUDescriptorHandleForHeapStart();
		mStride = device->GetDescriptorHandleIncrementSize(desc.Type);
		mCount = desc.NumDescriptors;
		mType = desc.Type;
	}

	DescriptorHeap(const DescriptorHeap&) = delete;
	DescriptorHeap& operator=(const DescriptorHeap&) = delete;
	DescriptorHeap(DescriptorHeap&&) noexcept = default;
	DescriptorHeap& operator=(DescriptorHeap&&) noexcept = default;

	inline D3D12_CPU_DESCRIPTOR_HANDLE descriptor_begin() const noexcept
	{
		return mBegin;
	}

	inline D3D12_CPU_DESCRIPTOR_HANDLE descriptor_at(SIZE_T index) const noexcept 
	{
		return D3D12_CPU_DESCRIPTOR_HANDLE{ mBegin.ptr + index * mStride }; 
	}

	inline UINT stride() const noexcept { return mStride; }
	inline UINT count() const noexcept  { return mCount; }
	inline D3D12_DESCRIPTOR_HEAP_TYPE type() const noexcept { return mType; }

private:

	Microsoft::WRL::ComPtr<ID3D12DescriptorHeap>   mHeap;
	UINT                                           mStride;
	UINT                                           mCount;
	D3D12_DESCRIPTOR_HEAP_TYPE                     mType;

	D3D12_CPU_DESCRIPTOR_HANDLE                    mBegin;
};