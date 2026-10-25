#pragma once

#include <d3d12.h>
#include <wrl/client.h>

class DescriptorHeap final
{
public:

	DescriptorHeap(ID3D12Device4* device, const D3D12_DESCRIPTOR_HEAP_DESC& desc);

	D3D12_CPU_DESCRIPTOR_HANDLE handle(UINT index = 0) const noexcept {
		return D3D12_CPU_DESCRIPTOR_HANDLE{ mBegin.ptr + (index * mStride) };
	}

	UINT stride() const noexcept {
		return mStride;
	}

	UINT size() const noexcept {
		return mSize;
	}

	D3D12_DESCRIPTOR_HEAP_TYPE type() const noexcept {
		return mType;
	}

private:

	Microsoft::WRL::ComPtr<ID3D12DescriptorHeap>   mHeap;
	UINT                                           mStride;
	UINT                                           mSize;
	D3D12_DESCRIPTOR_HEAP_TYPE                     mType;

	D3D12_CPU_DESCRIPTOR_HANDLE                    mBegin;
};