#pragma once

#include "GPU_CORE.h"
#include "Utils.h"
#include <chrono>

class DX12Fence
{
public:
	DX12Fence() :fence(nullptr), command_queue(nullptr), event_handle(nullptr), counter(0) {}

	void initialize(DXDevice2& device, DXCommandQueue& queue, D3D12_FENCE_FLAGS flags)
	{
		if (fence)
			throw std::runtime_error("reinitialization error");
		if (!queue)
			throw std::runtime_error{"nullptr queue"};

		execute_test_throw(
			device->CreateFence(0ull, flags, IID_PPV_ARGS(&fence))
		);

		event_handle = ::CreateEvent(NULL, FALSE, FALSE, NULL);
		if (!event_handle)
			throw_error_code_translation(GetLastError());

		command_queue = queue;
	}

	// signals the GPU with a value and returns that value for the user to handle
	UINT64 signal()
	{
		UINT64 signaled_value = ++counter;

		execute_test_throw(
			command_queue->Signal(fence.Get(), signaled_value)
		);

		return signaled_value;
	}

	// checks if the internal fence value has reached the expected value. if not stalls the CPU
	void wait(UINT64 expected_value, std::chrono::milliseconds duration = std::chrono::milliseconds::max())
	{
		// check if waiting is required at all
		if (fence->GetCompletedValue() < expected_value)
		{
			// trigger event when value is reached
			execute_test_throw(
				fence->SetEventOnCompletion(expected_value, event_handle)
			);
			// stall the CPU thread
			const DWORD wait_ms = (duration == std::chrono::milliseconds::max()) ? INFINITE : static_cast<DWORD>(duration.count());
			::WaitForSingleObject(event_handle, wait_ms);
		}
	}

	DX12Fence(const DX12Fence&) = delete;
	DX12Fence& operator=(const DX12Fence&) = delete;
	DX12Fence(DX12Fence&&)noexcept = delete;
	DX12Fence& operator=(DX12Fence&&)noexcept = delete;

	~DX12Fence()
	{
		if (event_handle)
			::CloseHandle(event_handle);
	}

private:
	DXFence fence;
	DXCommandQueue command_queue;
	UINT64 counter;
	HANDLE event_handle;
};