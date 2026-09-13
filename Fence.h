#pragma once

#include <d3d12.h>
#include <wrl/client.h>

class Fence final
{
public:

	Fence() = default;
	Fence(ID3D12Device4* device, UINT64 initial_value, D3D12_FENCE_FLAGS flags = D3D12_FENCE_FLAG_NONE);
	~Fence() noexcept;

	Fence(const Fence&) = delete;
	Fence& operator=(const Fence&) = delete;

	Fence(Fence&&) noexcept;
	Fence& operator=(Fence&&) noexcept;


	UINT64 get_completed_value() const;
	void   set_event(UINT64 value);
	void   wait_event(DWORD milliseconds = INFINITE) const;
	void   wait(UINT64 value, DWORD milliseconds = INFINITE);
	void   manual_signal(UINT64 value);

	ID3D12Fence* get() const noexcept;
private:
	Microsoft::WRL::ComPtr<ID3D12Fence> mFence       = nullptr;
	HANDLE                              mEventHandle = nullptr;
};