#pragma once
#include "badDirectX.h"
#include <wrl/client.h>
#include "ViewPtr.h"

class Fence final
{
public:

	Fence(ViewPtr<ID3D12Device4> device);

	Fence(const Fence&) = delete;
	Fence& operator=(const Fence&) = delete;
	Fence(Fence&&) noexcept = delete;
	Fence& operator=(Fence&&) noexcept = delete;

	virtual ~Fence() noexcept;

	UINT64 get_completed_value() const;

	UINT64 signal(ViewPtr<ID3D12CommandQueue> command_queue);

	void wait(UINT64 expected_value);

private:
	Microsoft::WRL::ComPtr<ID3D12Fence> mFence       = nullptr;
	HANDLE                              mEventHandle = nullptr;
	UINT64                              mCounter     = 0;
};