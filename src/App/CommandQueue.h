#pragma once

#include <mutex>
#include <atomic>
#include <memory>
#include <queue>
#include <vector>

#include <d3d12.h>
#include <wrl/client.h>

#include "Tools/ViewPtr.h"
#include "D3D12/FenceView.h"

// todo:: submit does not check if submitted list is of the same type as the queue... fix it

class CommandList;

class CommandQueue final
{
public:

	CommandQueue() = delete;
	CommandQueue(ID3D12Device4* device, D3D12_COMMAND_LIST_TYPE type);
	~CommandQueue();

	CommandQueue(const CommandQueue&) = delete;
	CommandQueue& operator=(const CommandQueue&) = delete;
	CommandQueue(CommandQueue&&) noexcept = delete;
	CommandQueue& operator=(CommandQueue&&) noexcept = delete;

	void submit(std::unique_ptr<CommandList> list);

	UINT64 execute();

	bool wait_until_completion(UINT64 until, DWORD milliseconds = INFINITE);

	void flush();

	std::unique_ptr<CommandList> get_command_list();

	ID3D12CommandQueue* get_queue() const noexcept { 
		return mCommandQueue.Get(); 
	}

	FenceView get_fence_view() const noexcept {
		return FenceView{ mFence.Get(), mFenceValue };
	}

protected:

	UINT64 signal();
	
private:

	struct CommandListEntry
	{
		UINT64                        fence_value = 0ull;
		std::unique_ptr<CommandList>  list = nullptr;
	};

	using InFlightListQueue = std::queue<CommandListEntry>;
	using SubmissionVector = std::vector<std::unique_ptr<CommandList>>;

	Microsoft::WRL::ComPtr<ID3D12CommandQueue> mCommandQueue;
	D3D12_COMMAND_LIST_TYPE mType;
	ViewPtr<ID3D12Device4> mDevice;

	Microsoft::WRL::ComPtr<ID3D12Fence> mFence;
	std::atomic<UINT64> mFenceValue;
	std::mutex mLock;

	InFlightListQueue mFlightQueue;
	SubmissionVector mSubmitList;
};