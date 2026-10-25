#pragma once

#include <string>

#include <d3d12.h>
#include <wrl/client.h>

// todo: root signature builder..., this class just loads ready made root sig

class RootSignature
{
public:
	RootSignature(ID3D12Device* device, const std::wstring& file);


	ID3D12RootSignature* get() const { return mRootSig.Get(); }

	UINT num_descriptors(uint32_t rootIndex) const { return mNumDescriptorsPerTable[rootIndex]; }
	UINT table_bitmap() const { return mDescriptorTableBitMap; }
	UINT sampler_bitmap() const { return mSamplerTableBitMap; }

	UINT parameter_count() const { return mParameterCount; }

private:
	Microsoft::WRL::ComPtr<ID3D12RootSignature> mRootSig;

	UINT mNumDescriptorsPerTable[32] = {0};
	UINT mDescriptorTableBitMap = 0;
	UINT mSamplerTableBitMap = 0;

	UINT mParameterCount = 0;
};