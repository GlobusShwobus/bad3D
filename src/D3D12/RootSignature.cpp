#include "D3D12/RootSignature.h"

#include <stdexcept>
#include <d3dcompiler.h>
#include <assert.h>

#include "D3D12/EasyDirectXUtils.h"

RootSignature::RootSignature(ID3D12Device* device, const std::wstring& file)
{
	if (!device)
		throw std::invalid_argument{ "Device nullptr" };

	Microsoft::WRL::ComPtr<ID3DBlob> blob;

	execute_and_test_hresult(
		D3DReadFileToBlob(L"DemoCubeRootSig.bin", &blob)
	);

	// create the root signature from read blob
	execute_and_test_hresult(
		device->CreateRootSignature(
			0, 
			blob->GetBufferPointer(),
			blob->GetBufferSize(), 
			IID_PPV_ARGS(&mRootSig)
		)
	);

	// read the layout of the blob
	Microsoft::WRL::ComPtr<ID3D12VersionedRootSignatureDeserializer> deser;
	D3D12CreateVersionedRootSignatureDeserializer(
		blob->GetBufferPointer(),
		blob->GetBufferSize(),
		IID_PPV_ARGS(&deser)
	);

	// extract root sig desc
	const D3D12_VERSIONED_ROOT_SIGNATURE_DESC* versioned = nullptr;
	deser->GetRootSignatureDescAtVersion(D3D_ROOT_SIGNATURE_VERSION_1_1, &versioned);
	const D3D12_ROOT_SIGNATURE_DESC1& desc = versioned->Desc_1_1;

	if (desc.NumParameters > 32)
		throw std::invalid_argument{"invalid root parameter count"};

	mParameterCount = desc.NumParameters;

	// iterate over the whole top level root signature description
	// the class only caches info if the parameter is a table, otherwise descriptors are inlined
	// the positions of tables in the parameter array are stored as bits in bit maps (the tables may also be 2 different kinds)
	for (UINT param = 0; param < desc.NumParameters; param++)
	{
		assert(desc.pParameters != nullptr);
		
		const D3D12_ROOT_PARAMETER1& root_param = desc.pParameters[param];

		if (root_param.ParameterType != D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE)
			continue; // inline CBV/SRV/UAV and root constants do no need descriptors

		const D3D12_ROOT_DESCRIPTOR_TABLE1& root_table = root_param.DescriptorTable;
		assert(root_table.pDescriptorRanges != nullptr);

		// keep track of sampler descriptor tables separately from CBV_SRV_UAV descriptor tables
		if (root_table.pDescriptorRanges->RangeType == D3D12_DESCRIPTOR_RANGE_TYPE_SAMPLER)
			mSamplerTableBitMap |= (1u << param);
		else
			mDescriptorTableBitMap |= (1u << param);

		for (UINT table_range = 0; table_range < root_table.NumDescriptorRanges; ++table_range) {
			// the description allocations are contiguous, but in case a fuck up happens in the future or something gets redesigned
			assert(root_table.pDescriptorRanges->OffsetInDescriptorsFromTableStart == D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND);
			mNumDescriptorsPerTable[param] += root_table.pDescriptorRanges[table_range].NumDescriptors;
		}
	}
}
