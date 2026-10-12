#pragma once

#include <stdexcept>
#include <atomic>

#include <d3d12.h>

#include "Tools/ViewPtr.h"

class FenceView
{
public:
	FenceView() = delete;

	FenceView(ID3D12Fence* fence, const std::atomic<UINT64>& counter)
		: mView(fence), mSignalCounter(counter)
	{
		if (!mView)
			throw std::invalid_argument("Fence nullptr");
	}

	auto get_completed_value() const {
		return mView->GetCompletedValue();
	}

	UINT64 get_next_signal_value() const noexcept {
		return mSignalCounter.load() + 1;
	}

private:

	ViewPtr<ID3D12Fence> mView;
	const std::atomic<UINT64>& mSignalCounter;
};
