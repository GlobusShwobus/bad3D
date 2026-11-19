#include "D3D12/DescriptorHeap.h"

#include <stdexcept>

#include "D3D12/EasyDirectXUtils.h"

DescriptorHeap::DescriptorHeap(ID3D12Device4* device, const D3D12_DESCRIPTOR_HEAP_DESC& desc)
{
	if (!device)
		throw std::invalid_argument{ "nullptr device" };

	execute_and_test_hresult(
		device->CreateDescriptorHeap(&desc, IID_PPV_ARGS(&mHeap))
	);

	mStride = device->GetDescriptorHandleIncrementSize(desc.Type);
	mSize = desc.NumDescriptors;
	mType = desc.Type;

	mIsShaderVisible =
		(desc.Type == D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV || desc.Type == D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER) &&
		(desc.Flags & D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE);
}