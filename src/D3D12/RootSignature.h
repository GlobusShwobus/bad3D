#pragma once

#include <string>
#include <vector>

#include <wrl/client.h>
#include <d3d12.h>
#include <d3dcompiler.h>

#include "D3D12/Globals.h"

struct SerializedRootSignatureProduct
{
	Microsoft::WRL::ComPtr<ID3D12RootSignature> root_signature;
	Microsoft::WRL::ComPtr<ID3DBlob> root_blob;
};



class RootSignature
{
public:
	RootSignature(ID3D12Device* device, const std::wstring& file);

	RootSignature(SerializedRootSignatureProduct serialized_root_signature);

	ID3D12RootSignature* get() const noexcept{
		return mRootSig.Get(); 
	}

	UINT CBV_SRV_UAV_table_bitmap () const noexcept {
		return mDescriptorTableBitMap; 
	}

	UINT sampler_bitmap() const noexcept { 
		return mSamplerTableBitMap; 
	}

	UINT parameter_count() const noexcept { 
		return mParameterCount; 
	}

	UINT num_descriptors_for_table(uint32_t root_index) const noexcept;

protected:

	void deserialized_info_extraction(const Microsoft::WRL::ComPtr<ID3DBlob>& blob);

private:
	Microsoft::WRL::ComPtr<ID3D12RootSignature> mRootSig;

	UINT mNumDescriptorsPerTable[Globals::ROOT_PARAMETER_LIMIT];
	UINT mDescriptorTableBitMap;
	UINT mSamplerTableBitMap;

	UINT mParameterCount;
};



class RootSignatureBuilder
{
public:

	RootSignatureBuilder()
		:mDwordCost(0)
	{}

	UINT add_constants(D3D12_SHADER_VISIBILITY shader_visibility, UINT shader_register, UINT num32BitValues, UINT register_space = 0U);
	UINT add_constants(const D3D12_ROOT_PARAMETER1& constant);

	UINT add_descriptor(D3D12_ROOT_PARAMETER_TYPE type, D3D12_SHADER_VISIBILITY shader_visibility, UINT shader_register, UINT register_space = 0U, D3D12_ROOT_DESCRIPTOR_FLAGS flags = D3D12_ROOT_DESCRIPTOR_FLAG_NONE);
	UINT add_descriptor(const D3D12_ROOT_PARAMETER1& descriptor);

	UINT add_table(D3D12_SHADER_VISIBILITY shader_visibility, const std::vector<D3D12_DESCRIPTOR_RANGE1>& ranges);

	void add_static_sampler(const D3D12_STATIC_SAMPLER_DESC& sampler);

	UINT signature_cost() const noexcept {
		return mDwordCost;
	}

	UINT max_signature_cost() const noexcept {
		return Globals::ROOT_DWORD_LIMIT;
	}

	bool is_limit_reached() const noexcept {
		return signature_cost() == max_signature_cost();
	}

	SerializedRootSignatureProduct build(ID3D12Device* device, const std::wstring& name, D3D12_ROOT_SIGNATURE_FLAGS flags) const;

protected:

	UINT add_param(const D3D12_ROOT_PARAMETER1& param, UINT index = Globals::NOT_ROOT_TABLE);

private:

	struct RootParam {
		D3D12_ROOT_PARAMETER1 param;
		UINT table_index;
	};

	using RootRange = std::vector<D3D12_DESCRIPTOR_RANGE1>;

	std::vector<RootParam> mParams;
	std::vector<RootRange> mRanges;
	std::vector<D3D12_STATIC_SAMPLER_DESC> mStaticSamplers;

	UINT mDwordCost;
};



void write_blob_to_file(ID3DBlob* blob, const std::wstring& path);

static UINT parameter_dword_cost(const D3D12_ROOT_PARAMETER1& param)
{
	switch (param.ParameterType)
	{
	case D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE:
		return 1;
	case D3D12_ROOT_PARAMETER_TYPE_32BIT_CONSTANTS:
		return param.Constants.Num32BitValues;
	case D3D12_ROOT_PARAMETER_TYPE_CBV:
	case D3D12_ROOT_PARAMETER_TYPE_SRV:
	case D3D12_ROOT_PARAMETER_TYPE_UAV:
		return 2;
	}
	return 0; // never happens but...
}