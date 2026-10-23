#include "D3D12/RootSignature.h"

#include <stdexcept>
#include <d3dcompiler.h>

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

	// maybe throw is size is over 32...
	for (UINT i = 0; i < desc.NumParameters && i < 32; i++) 
	{
		const D3D12_ROOT_PARAMETER1& p = desc.pParameters[i];

		if (p.ParameterType != D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE)
			continue; // inline CBV/SRV/UAV and root constants do no need descriptors

		const auto& table = p.DescriptorTable;

		UINT count = 0;
		for (UINT r = 0; r < table.NumDescriptorRanges; r++)
		{
			count += table.pDescriptorRanges[r].NumDescriptors;
		}

		// D3D12 forbits mixing samplers and non-samplers in one table
		if (table.pDescriptorRanges[0].RangeType == D3D12_DESCRIPTOR_RANGE_TYPE_SAMPLER)
			mSamplerTableBitMask |= (1u << i);
		else
			mTableBitMask |= (1u << i);

		mNumDescriptorsPerTable[i] = count;
	}
}
