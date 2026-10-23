#pragma once

#include <string>

#include <d3d12.h>
#include <wrl/client.h>

class RootSignature
{
public:
	RootSignature(ID3D12Device* device, const std::wstring& file);


	ID3D12RootSignature* get() const { return mRootSig.Get(); }

private:
	Microsoft::WRL::ComPtr<ID3D12RootSignature> mRootSig;

	UINT mNumDescriptorsPerTable[32] = {};
	UINT mTableBitMask = 0;
	UINT mSamplerTableBitMask = 0;
};