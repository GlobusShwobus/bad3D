#pragma once

#include "GPU_CORE.h"
#include "ObserverPtr.h"
#include "DX12Fence.h"
#include <queue>

class DX12CommandQueue final
{
public:
	DX12CommandQueue(ObserverPtr<ID3D12Device2> device, D3D12_COMMAND_QUEUE_DESC desc);
	DX12CommandQueue(const DX12CommandQueue&) = delete;
	DX12CommandQueue& operator=(const DX12CommandQueue&) = delete;
	DX12CommandQueue(DX12CommandQueue&&) = delete;
	DX12CommandQueue& operator=(DX12CommandQueue&&) = delete;
	virtual ~DX12CommandQueue();

	// get an available command list from command queue
	D3D12GraphicsCommandList2 get_command_list();

	// get internal command queue, for swap list
	constexpr ObserverPtr<ID3D12CommandQueue> get_observer()const noexcept { return  mCommandQueue.Get(); }

	// execute a command list. retruns the fence value to wait for
	UINT64 execute(D3D12GraphicsCommandList2 command_list);

	// forces a CPU stall
	void flush();

	// checks if expected value is more than fence value. if fence value is less, it will stall the CPU, otherwise nothing
	void wait(UINT64 expected_value);

protected:

	UINT64 signal();

	D3D12CommandAllocator create_command_allocator();

	D3D12GraphicsCommandList2 create_command_list(ObserverPtr<ID3D12CommandAllocator> command_allocator);

private:

	// keep track of command allocators that are executed
	struct CommandAllocatorEntry
	{
		UINT64 fence_value;
		D3D12CommandAllocator command_allocator;
	};

	ObserverPtr<ID3D12Device2>         mDevice;
	D3D12CommandQueue                  mCommandQueue;

	DX12Fence mFenceHandle;

	const D3D12_COMMAND_LIST_TYPE mType;

	std::queue<CommandAllocatorEntry>            mCommandAllocatorQueue;
	std::queue<D3D12GraphicsCommandList2>        mCommandListQueue;
};