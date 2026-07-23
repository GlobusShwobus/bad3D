#include "DX12Fence.h"
#include "Utils.h"
#include <utility>

DX12Fence::DX12Fence(ObserverPtr<ID3D12Device2> device)
	:mCounter(0ull), mFence(nullptr), mEventHandle(nullptr)
{
	if (!device)
		throw_error_code_translation(static_cast<DWORD>(E_POINTER));

	execute_test_throw(
		device->CreateFence(mCounter, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&mFence))
	);

	mEventHandle = ::CreateEventW(NULL, FALSE, FALSE, NULL);
	if (!mEventHandle)
		throw_error_code_translation(GetLastError());
}

DX12Fence::DX12Fence(DX12Fence&& rhs) noexcept
	:mFence(std::move(rhs.mFence)), mCounter(rhs.mCounter), mEventHandle(rhs.mEventHandle)
{
	rhs.mEventHandle = nullptr;
	rhs.mCounter = 0;
}

DX12Fence& DX12Fence::operator=(DX12Fence&& rhs) noexcept
{
	if (this != &rhs)
	{
		mFence = std::move(rhs.mFence);
		mCounter = rhs.mCounter;
		rhs.mCounter = 0;

		if (mEventHandle)
		{
			::CloseHandle(mEventHandle);
		}
		mEventHandle = rhs.mEventHandle;
		rhs.mEventHandle = nullptr;
	}
	return *this;
}

DX12Fence::~DX12Fence() noexcept
{
	if (mEventHandle)
		::CloseHandle(mEventHandle);
	mEventHandle = nullptr;
}

UINT64 DX12Fence::get_completed_value() const noexcept { return mFence->GetCompletedValue(); }

void DX12Fence::stall_thread_until(UINT64 expected_value)
{
	// trigger event when value is reached
	execute_test_throw(
		mFence->SetEventOnCompletion(expected_value, mEventHandle)
	);
	// stall the CPU thread
	::WaitForSingleObject(mEventHandle, INFINITE);
}