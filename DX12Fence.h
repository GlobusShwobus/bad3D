#pragma once
#include "GPU_CORE.h"
#include "ObserverPtr.h"

class DX12Fence
{
public:

	DX12Fence(ObserverPtr<ID3D12Device2> device);

	DX12Fence(const DX12Fence&) = delete;
	DX12Fence& operator=(const DX12Fence&) = delete;

	DX12Fence(DX12Fence&& rhs) noexcept;
	DX12Fence& operator=(DX12Fence&& rhs) noexcept;

	virtual ~DX12Fence() noexcept;

	UINT64 get_completed_value() const noexcept;
	constexpr UINT64 get_counter_value() const noexcept { return mCounter; }
	constexpr void increment_counter() noexcept { mCounter++; }
	constexpr ObserverPtr<ID3D12Fence> get_observer()const noexcept { return ObserverPtr<ID3D12Fence>{mFence.Get()}; }

	void stall_thread_until(UINT64 expected_value);

private:
	D3D12Fence mFence;
	UINT64  mCounter;
	HANDLE  mEventHandle;
};