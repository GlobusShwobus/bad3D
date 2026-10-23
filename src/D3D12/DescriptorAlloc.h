#pragma once

#include <d3d12.h>

#include "Tools/ViewPtr.h"

class DescriptorAlloc
{
public:

	constexpr DescriptorAlloc() noexcept
		:mStride(0)
		, mSize(0)
		, mBegin(0)
		, mSource(nullptr)
	{
	}

	~DescriptorAlloc();

	DescriptorAlloc(const DescriptorAlloc&) = delete;
	DescriptorAlloc& operator=(const DescriptorAlloc&) = delete;

	DescriptorAlloc(DescriptorAlloc&& rhs) noexcept
		: mBegin(rhs.mBegin), mSize(rhs.mSize), mStride(rhs.mStride), mSource(rhs.mSource)
	{
		rhs.mBegin.ptr = 0;
		rhs.mSize = 0;
		rhs.mStride = 0;
		rhs.mSource = nullptr;
	}

	DescriptorAlloc& operator=(DescriptorAlloc&& rhs) noexcept;

	bool is_null() const noexcept {
		return mBegin.ptr == 0;
	}

	// Get a descriptor at a particular offset in the allocation.
	D3D12_CPU_DESCRIPTOR_HANDLE handle(UINT offset = 0) const noexcept {
		return { mBegin.ptr + (mStride * offset) };
	}

	// Get the number of (consecutive) handles for this allocation.
	UINT size() const noexcept {
		return mSize;
	}

protected:

	friend class DescriptorPage;

	DescriptorAlloc(
		D3D12_CPU_DESCRIPTOR_HANDLE descriptor,
		UINT num_handles,
		UINT stride,
		DescriptorPage* source
	);

	void free();

private:

	D3D12_CPU_DESCRIPTOR_HANDLE mBegin;
	UINT mSize;
	UINT mStride;

	// A pointer back to the original page where this allocation came from.
	ViewPtr<DescriptorPage> mSource;
};