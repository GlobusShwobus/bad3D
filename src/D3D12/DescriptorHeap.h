#pragma once

#include <d3d12.h>
#include <wrl/client.h>

#include <assert.h>

class DescriptorHeap final
{
public:

	DescriptorHeap(ID3D12Device4* device, const D3D12_DESCRIPTOR_HEAP_DESC& desc);

	D3D12_CPU_DESCRIPTOR_HANDLE cpu_handle() const noexcept {
		return mHeap->GetCPUDescriptorHandleForHeapStart();
	}
	
	D3D12_GPU_DESCRIPTOR_HANDLE gpu_handle() const noexcept {
		assert(mIsShaderVisible);
		return mHeap->GetGPUDescriptorHandleForHeapStart();
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
	bool                                           mIsShaderVisible;
};
