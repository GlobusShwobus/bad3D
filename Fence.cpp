#include "Fence.h"
#include "Utils.h"
#include <utility>

Fence::Fence(ViewPtr<ID3D12Device4> device)
{
	assert(device && "device nullptr");

	execute_and_test_hresult(
		device->CreateFence(mCounter, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&mFence))
	);

	mEventHandle = ::CreateEventW(NULL, FALSE, FALSE, NULL);
	if (!mEventHandle)
		throw_error_code_translation(GetLastError());
}

Fence::~Fence() noexcept
{
	if (mEventHandle)
		::CloseHandle(mEventHandle);
	mEventHandle = nullptr;
	mFence.Reset();
}

UINT64 Fence::get_completed_value() const
{
	return mFence->GetCompletedValue(); 
}

UINT64 Fence::signal(ViewPtr<ID3D12CommandQueue> command_queue)
{
	assert(command_queue && "command_queue nullptr");

	UINT64 signaled_value = mCounter;
	mCounter++;

	execute_and_test_hresult(
		command_queue->Signal(mFence.Get(), signaled_value)
	);

	return signaled_value;
}

void Fence::wait(UINT64 expected_value)
{
	// trigger event when value is reached
	execute_and_test_hresult(
		mFence->SetEventOnCompletion(expected_value, mEventHandle)
	);
	// stall the CPU thread
	::WaitForSingleObject(mEventHandle, INFINITE);
}
