#pragma once

#include <memory>

#include <d3d12.h>
#include <wrl/client.h>

class DescriptorHeap final
{
public:
	DescriptorHeap() = delete;

	static std::unique_ptr<DescriptorHeap> create(ID3D12Device4* device, const D3D12_DESCRIPTOR_HEAP_DESC& desc)
	{
		if (!device)
			return nullptr;

		Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> heap;
		if (FAILED(device->CreateDescriptorHeap(&desc, IID_PPV_ARGS(&heap))))
			return nullptr;

		UINT stride = device->GetDescriptorHandleIncrementSize(desc.Type);
		D3D12_CPU_DESCRIPTOR_HANDLE begin = heap->GetCPUDescriptorHandleForHeapStart();

		// if new fails
		try {
			return std::unique_ptr<DescriptorHeap>(new DescriptorHeap(std::move(heap), stride, desc.NumDescriptors, desc.Type, begin));
		}
		catch (const std::bad_alloc&) {
			return nullptr;
		}
	}

	// dont wanna think about it atm
	DescriptorHeap(const DescriptorHeap&) = delete;
	DescriptorHeap& operator=(const DescriptorHeap&) = delete;
	DescriptorHeap(DescriptorHeap&&) noexcept = default;
	DescriptorHeap& operator=(DescriptorHeap&&) noexcept = default;

	constexpr D3D12_CPU_DESCRIPTOR_HANDLE descriptor_begin() const noexcept { return mBegin; }
	constexpr D3D12_CPU_DESCRIPTOR_HANDLE descriptor_at(SIZE_T index) const noexcept { return D3D12_CPU_DESCRIPTOR_HANDLE{ mBegin.ptr + index * mStride }; }

	constexpr UINT stride() const noexcept { return mStride; }
	constexpr UINT count() const noexcept  { return mCount; }
	constexpr D3D12_DESCRIPTOR_HEAP_TYPE type() const noexcept { return mType; }

private:

	DescriptorHeap(
		Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> heap,
		UINT stride,
		UINT count,
		D3D12_DESCRIPTOR_HEAP_TYPE type,
		D3D12_CPU_DESCRIPTOR_HANDLE begin
	) noexcept
		: mHeap(std::move(heap)), mStride(stride), mCount(count), mType(type), mBegin(begin) {
	}


	Microsoft::WRL::ComPtr<ID3D12DescriptorHeap>   mHeap = nullptr;
	UINT                                           mStride = 0;
	UINT                                           mCount = 0;
	D3D12_DESCRIPTOR_HEAP_TYPE                     mType{};

	D3D12_CPU_DESCRIPTOR_HANDLE                    mBegin{};
};