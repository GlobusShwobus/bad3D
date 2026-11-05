#include "D3D12/RootSignature.h"

#include <assert.h>
#include <stdexcept>

#include "D3D12/EasyDirectXUtils.h"


void write_blob_to_file(ID3DBlob* blob, const std::wstring& path)
{
	if (!blob)
		throw std::invalid_argument{ "blob nullptr" };

	execute_and_test_hresult(
		D3DWriteBlobToFile(blob, path.c_str(), TRUE)
	);
}

RootSignature::RootSignature(ID3D12Device* device, const std::wstring& file)
	:mNumDescriptorsPerTable({})
	, mDescriptorTableBitMap(0)
	, mSamplerTableBitMap(0)
	, mParameterCount(0)
{
	if (!device)
		throw std::invalid_argument{ "Device nullptr" };

	Microsoft::WRL::ComPtr<ID3DBlob> blob;

	execute_and_test_hresult(
		D3DReadFileToBlob(file.c_str(), &blob)
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

	deserialized_info_extraction(blob);
}

RootSignature::RootSignature(SerializedRootSignatureProduct serialized_root_signature)
	:mRootSig(serialized_root_signature.root_signature)
	, mNumDescriptorsPerTable({})
	, mDescriptorTableBitMap(0)
	, mSamplerTableBitMap(0)
	, mParameterCount(0)
{
	if (!mRootSig)
		throw std::invalid_argument{ "nullptr root sig" };

	deserialized_info_extraction(serialized_root_signature.root_blob);
}

UINT RootSignature::num_descriptors_for_table(uint32_t root_index) const noexcept
{
	assert(root_index < mParameterCount);
	return mNumDescriptorsPerTable[root_index];
}

void RootSignature::deserialized_info_extraction(const Microsoft::WRL::ComPtr<ID3DBlob>& blob)
{
	if (!blob)
		throw std::invalid_argument{ "nullptr blob" };

	// read the layout of the blob
	Microsoft::WRL::ComPtr<ID3D12VersionedRootSignatureDeserializer> deser;
	execute_and_test_hresult(
		D3D12CreateVersionedRootSignatureDeserializer(
			blob->GetBufferPointer(),
			blob->GetBufferSize(),
			IID_PPV_ARGS(&deser)
		)
	);

	// extract root sig desc
	const D3D12_VERSIONED_ROOT_SIGNATURE_DESC* versioned = nullptr;

	execute_and_test_hresult(
		deser->GetRootSignatureDescAtVersion(D3D_ROOT_SIGNATURE_VERSION_1_1, &versioned)
	);

	const D3D12_ROOT_SIGNATURE_DESC1& desc = versioned->Desc_1_1;

	if (desc.NumParameters > Globals::ROOT_PARAMETER_LIMIT)
		throw std::invalid_argument{ "invalid root parameter count" };


	mParameterCount = desc.NumParameters;
	mSamplerTableBitMap = 0;
	mDescriptorTableBitMap = 0;

	// iterate over the whole top level root signature description
	// the class only caches info if the parameter is a table, otherwise descriptors are inlined
	// the positions of tables in the parameter array are stored as bits in bit maps (the tables may also be 2 different kinds)
	for (UINT param = 0; param < mParameterCount; param++)
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

		// !!! the assert in the loop. by deign it should always be appent BUT who tf knows where the roads go so keep in mind

		for (UINT table_range = 0; table_range < root_table.NumDescriptorRanges; ++table_range) {
			// the description allocations are contiguous, but in case a fuck up happens in the future or something gets redesigned
			assert(root_table.pDescriptorRanges[table_range].OffsetInDescriptorsFromTableStart == D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND);
			mNumDescriptorsPerTable[param] += root_table.pDescriptorRanges[table_range].NumDescriptors;
		}
	}
}

UINT RootSignatureBuilder::add_constants(D3D12_SHADER_VISIBILITY shader_visibility, UINT shader_register, UINT num32BitValues, UINT register_space)
{
	D3D12_ROOT_PARAMETER1 desc{};
	desc.ParameterType = D3D12_ROOT_PARAMETER_TYPE_32BIT_CONSTANTS;
	desc.ShaderVisibility = shader_visibility;
	desc.Constants.Num32BitValues = num32BitValues;
	desc.Constants.ShaderRegister = shader_register;
	desc.Constants.RegisterSpace = register_space;

	return add_constants(desc);
}

UINT RootSignatureBuilder::add_constants(const D3D12_ROOT_PARAMETER1& constant)
{
	if (constant.ParameterType != D3D12_ROOT_PARAMETER_TYPE_32BIT_CONSTANTS)
		throw std::invalid_argument{ "invalid parameter type" };

	return add_param(constant);
}

UINT RootSignatureBuilder::add_descriptor(D3D12_ROOT_PARAMETER_TYPE type, D3D12_SHADER_VISIBILITY shader_visibility, UINT shader_register, UINT register_space, D3D12_ROOT_DESCRIPTOR_FLAGS flags)
{
	D3D12_ROOT_PARAMETER1 desc{};
	desc.ParameterType = type;
	desc.ShaderVisibility = shader_visibility;
	desc.Descriptor.RegisterSpace = register_space;
	desc.Descriptor.ShaderRegister = shader_register;
	desc.Descriptor.Flags = flags;
	return add_descriptor(desc);
}

UINT RootSignatureBuilder::add_descriptor(const D3D12_ROOT_PARAMETER1& descriptor)
{
	if (descriptor.ParameterType != D3D12_ROOT_PARAMETER_TYPE_CBV &&
		descriptor.ParameterType != D3D12_ROOT_PARAMETER_TYPE_SRV &&
		descriptor.ParameterType != D3D12_ROOT_PARAMETER_TYPE_UAV)
	{
		throw std::invalid_argument{ "invalid parameter type" };
	}
	return add_param(descriptor);
}

UINT RootSignatureBuilder::add_table(D3D12_SHADER_VISIBILITY shader_visibility, const std::vector<D3D12_DESCRIPTOR_RANGE1>& ranges)
{
	if (ranges.empty())
		throw std::invalid_argument{ "empty ranges not allowed" };

	const UINT index = static_cast<UINT>(mRanges.size());

	D3D12_ROOT_PARAMETER1 desc{};
	desc.ShaderVisibility = shader_visibility;
	desc.ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
	desc.DescriptorTable.NumDescriptorRanges = 0;					// they get added later
	desc.DescriptorTable.pDescriptorRanges = nullptr;				// they get added later

	// first try to add param, it if doesn't throw it's okey to push ranges
	const UINT graphics_root_param_index = add_param(desc, index);

	mRanges.push_back(ranges);

	return graphics_root_param_index;
}

void RootSignatureBuilder::add_static_sampler(const D3D12_STATIC_SAMPLER_DESC& sampler)
{
	// two static samplers may not share register+space with overlapping visibility
	for (const auto& e : mStaticSamplers)
	{
		const bool same_slot = e.ShaderRegister == sampler.ShaderRegister && e.RegisterSpace == sampler.RegisterSpace;
		const bool overlap = e.ShaderVisibility == D3D12_SHADER_VISIBILITY_ALL
			|| sampler.ShaderVisibility == D3D12_SHADER_VISIBILITY_ALL
			|| e.ShaderVisibility == sampler.ShaderVisibility;
		if (same_slot && overlap)
			throw std::invalid_argument{ "static sampler register/space already in use" };
	}
	mStaticSamplers.push_back(sampler);
}

SerializedRootSignatureProduct RootSignatureBuilder::build(ID3D12Device* device, const std::wstring& name, D3D12_ROOT_SIGNATURE_FLAGS flags) const
{
	if (!device)
		throw std::invalid_argument{ "Device nullptr" };

	// extract params data
	std::vector<D3D12_ROOT_PARAMETER1> param_array;
	param_array.reserve(mParams.size());

	for (const RootParam& param : mParams) {
		D3D12_ROOT_PARAMETER1 root_parameter1 = param.param;

		if (root_parameter1.ParameterType == D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE) {
			assert(param.table_index != Globals::NOT_ROOT_TABLE);
			const auto& range = mRanges[param.table_index];
			root_parameter1.DescriptorTable.NumDescriptorRanges = static_cast<UINT>(range.size());
			root_parameter1.DescriptorTable.pDescriptorRanges = range.data();
		}

		param_array.push_back(root_parameter1);
	}

	D3D12_ROOT_SIGNATURE_DESC1 root_desc{};
	root_desc.NumParameters = static_cast<UINT>(param_array.size());
	root_desc.pParameters = param_array.data();
	root_desc.NumStaticSamplers = static_cast<UINT>(mStaticSamplers.size());
	root_desc.pStaticSamplers = mStaticSamplers.data();
	root_desc.Flags = flags;

	// serialize
	D3D12_VERSIONED_ROOT_SIGNATURE_DESC versioned{};
	versioned.Version = D3D_ROOT_SIGNATURE_VERSION_1_1;
	versioned.Desc_1_1 = root_desc;

	Microsoft::WRL::ComPtr<ID3DBlob> blob, error;
	HRESULT hr = D3D12SerializeVersionedRootSignature(&versioned, &blob, &error);
	if (FAILED(hr) && error)
		OutputDebugStringA(static_cast<const char*>(error->GetBufferPointer()));

	execute_and_test_hresult(hr);

	Microsoft::WRL::ComPtr<ID3D12RootSignature> root_sig;
	execute_and_test_hresult(
		device->CreateRootSignature(0, blob->GetBufferPointer(), blob->GetBufferSize(), IID_PPV_ARGS(&root_sig))
	);

	root_sig->SetName(name.c_str());

	return SerializedRootSignatureProduct{
		std::move(root_sig),
		std::move(blob)
	};
}

UINT RootSignatureBuilder::add_param(const D3D12_ROOT_PARAMETER1& param, UINT index)
{
	if (mParams.size() >= Globals::ROOT_PARAMETER_LIMIT)
		throw std::length_error{ "more than 32 root parameters (bitmask limit)" };

	const UINT param_cost = parameter_dword_cost(param);
	if (mDwordCost + param_cost > Globals::ROOT_DWORD_LIMIT)
		throw std::length_error{ "root signature exceeds 64 DWORDs" };

	mDwordCost += param_cost;

	mParams.emplace_back(
		RootParam{ param, index }
	);

	return static_cast<UINT>(mParams.size() - 1);
}
