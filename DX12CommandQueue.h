#pragma once

#include "GPU_CORE.h"
#include "ObserverPtr.h"
#include "DX12Fence.h"
#include <queue>

class DX12CommandQueue final
{
	// keep track of command allocators that are executed
	struct CommandAllocatorQueueEntry
	{
		UINT64                fence_value;
		D3D12CommandAllocator command_allocator;
	};

	struct CommandContext
	{
		D3D12CommandAllocator     command_allocator;
		D3D12GraphicsCommandList2 command_list;
	};

	using QAllocEntry = std::queue<CommandAllocatorQueueEntry>;

public:

	DX12CommandQueue(D3D12_COMMAND_QUEUE_DESC desc, ObserverPtr<ID3D12Device4> device);
	
	DX12CommandQueue(const DX12CommandQueue&) = delete;
	DX12CommandQueue& operator=(const DX12CommandQueue&) = delete;
	DX12CommandQueue(DX12CommandQueue&&) = delete;
	DX12CommandQueue& operator=(DX12CommandQueue&&) = delete;

	// the destructor is not responsible for making sure if there is anything in execution in the background. the application must manually stall
	virtual ~DX12CommandQueue() = default;

	// execute a command list. retruns the fence value to wait for
	UINT64 execute();

	// forces a CPU stall
	void flush();

	// checks if expected value is more than fence value. if fence value is less, it will stall the CPU, otherwise nothing
	void wait(UINT64 expected_value);

	// get an available command list from command queue
	ObserverPtr<ID3D12GraphicsCommandList2> get_command_list();

	// get internal command queue, for swap list
	constexpr ObserverPtr<ID3D12CommandQueue> get_observer()const noexcept { return  mCommandQueue.Get(); }

protected:

	UINT64 signal();

	D3D12CommandAllocator create_command_allocator();

private:

	const D3D12_COMMAND_LIST_TYPE      mType;
	ObserverPtr<ID3D12Device4>         mDevice;
	D3D12CommandQueue                  mCommandQueue;

	DX12Fence                          mFence;

	QAllocEntry                        mAllocatorQueue;
	CommandContext                     mCommandContext;
};