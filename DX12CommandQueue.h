#pragma once

#include "GPU_CORE.h"
#include "ObserverPtr.h"
#include <queue>

class DX12CommandQueue final
{
public:
	DX12CommandQueue(ObserverPtr<ID3D12Device2> device, D3D12_COMMAND_LIST_TYPE type);
	DX12CommandQueue(const DX12CommandQueue&) = delete;
	DX12CommandQueue& operator=(const DX12CommandQueue&) = delete;
	DX12CommandQueue(DX12CommandQueue&&) = delete;
	DX12CommandQueue& operator=(DX12CommandQueue&&) = delete;
	virtual ~DX12CommandQueue();

	// get an available command list from command queue
	DXCommandList2 get_command_list();

	// execute a command list. retruns the fence value to wait for
	UINT64 execute(DXCommandList2 command_list);

	void flush();

	ID3D12CommandQueue* get_private()const;

	UINT64 signal();

	// checks if the internal fence value has reached the expected value. if not stalls the CPU
	void wait(UINT64 expected_value);

	bool is_fence_complete(UINT64 value)const;

protected:

	DXCommandAllocator create_command_allocator();

	DXCommandList2 create_command_list(DXCommandAllocator command_allocator);

private:

	// keep track of command allocators that are executed
	struct CommandAllocatorEntry
	{
		UINT64 fence_value;
		DXCommandAllocator command_allocator;
	};

	ObserverPtr<ID3D12Device2>      mDevice;
	DXCommandQueue                  mCommandQueue;

	DXFence mFence;
	UINT64  mFenceSignalCounter;
	HANDLE  mFenceEventHandle;

	const D3D12_COMMAND_LIST_TYPE mType;

	std::queue<CommandAllocatorEntry> mCommandAllocatorQueue;
	std::queue<DXCommandList2>        mCommandListQueue;
};