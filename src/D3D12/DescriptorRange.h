#pragma once

#include <d3d12.h>

#include "Tools/ViewPtr.h"

class DescriptorRange
{
public:

	constexpr DescriptorRange() noexcept
		:mStride(0)
		, mSize(0)
		, mBegin(0)
		, mHeap(nullptr)
	{
	}

	~DescriptorRange();

	DescriptorRange(const DescriptorRange&) = delete;
	DescriptorRange& operator=(const DescriptorRange&) = delete;

	DescriptorRange(DescriptorRange&& rhs) noexcept
		: mBegin(rhs.mBegin), mSize(rhs.mSize), mStride(rhs.mStride), mHeap(rhs.mHeap)
	{
		rhs.mBegin.ptr = 0;
		rhs.mSize = 0;
		rhs.mStride = 0;
		rhs.mHeap = nullptr;
	}

	DescriptorRange& operator=(DescriptorRange&& rhs) noexcept;

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

	friend class DescriptorFreeListHeap;

	DescriptorRange(
		D3D12_CPU_DESCRIPTOR_HANDLE descriptor,
		UINT num_handles,
		UINT stride,
		DescriptorFreeListHeap* page
	);

	void free();

private:

	D3D12_CPU_DESCRIPTOR_HANDLE mBegin;
	UINT mSize;
	UINT mStride;

	// A pointer back to the original page where this allocation came from.
	ViewPtr<DescriptorFreeListHeap> mHeap;
};