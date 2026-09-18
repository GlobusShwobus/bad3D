#include "Fence.h"
#include "EasyDirectXUtils.h"
#include <assert.h>
#include <utility>

Fence::Fence(ID3D12Device4* device, UINT64 initial_value, D3D12_FENCE_FLAGS flags)
{
	assert(device && "device nullptr");

	execute_and_test_hresult(
		device->CreateFence(initial_value, flags, IID_PPV_ARGS(&mFence))
	);

	mEventHandle = ::CreateEventW(NULL, FALSE, FALSE, NULL);
	if (!mEventHandle)
		throw_error_code_translation(GetLastError());
}

Fence::~Fence() noexcept
{
	if (mEventHandle)
		::CloseHandle(mEventHandle);
}

Fence::Fence(Fence&& rhs) noexcept
	:mFence(std::move(rhs.mFence)), mEventHandle(rhs.mEventHandle)
{
	rhs.mEventHandle = nullptr;
}

Fence& Fence::operator=(Fence&& rhs) noexcept
{
	if (this != &rhs)
	{
		mFence = std::move(rhs.mFence);

		if(mEventHandle)
			::CloseHandle(mEventHandle);

		mEventHandle = std::exchange(rhs.mEventHandle, nullptr);
	}
	return *this;
}

UINT64 Fence::get_completed_value() const
{
	return mFence->GetCompletedValue(); 
}

void Fence::set_event(UINT64 value)
{
	execute_and_test_hresult(
		mFence->SetEventOnCompletion(value, mEventHandle)
	);
}

void Fence::wait_event(DWORD milliseconds) const
{
	::WaitForSingleObject(mEventHandle, milliseconds);
}

void Fence::wait(UINT64 value, DWORD milliseconds)
{
	if (get_completed_value() >= value)
		return;

	set_event(value);
	wait_event(milliseconds);
}

void Fence::manual_signal(UINT64 value)
{
	execute_and_test_hresult(
		mFence->Signal(value)
	);
}

ID3D12Fence* Fence::get()const noexcept
{
	return mFence.Get();
}