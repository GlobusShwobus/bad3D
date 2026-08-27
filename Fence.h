#pragma once
#include "badDirectX.h"
#include <wrl/client.h>
#include "ViewPtr.h"

class Fence final
{
public:

	Fence(ViewPtr<ID3D12Device4> device, UINT64 initial_value);

	Fence(const Fence&) = delete;
	Fence& operator=(const Fence&) = delete;
	Fence(Fence&&) noexcept = delete;
	Fence& operator=(Fence&&) noexcept = delete;

	virtual ~Fence() noexcept;

	UINT64 get_completed_value() const;
	void   set_event(UINT64 value);
	void   wait_event() const;
	void   wait(UINT64 value);
	void   manual_signal(UINT64 value);

	constexpr ViewPtr<ID3D12Fence> get() const noexcept { return mFence.Get(); }
private:
	Microsoft::WRL::ComPtr<ID3D12Fence> mFence       = nullptr;
	HANDLE                              mEventHandle = nullptr;
};