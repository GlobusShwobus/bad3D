#pragma once

#include <queue>

#include <d3d12.h>
#include <wrl/client.h>

#include "D3D12/Fence.h"
#include "Tools/ViewPtr.h"

#include "CommandList.h"

class CommandQueue final
{
	// keep track of command allocators that are executed
	struct CommandAllocatorEntry
	{
		UINT64                                         fence_value       = 0ull;
		Microsoft::WRL::ComPtr<ID3D12CommandAllocator> command_allocator = nullptr;
	};

	using QAllocEntry = std::queue<CommandAllocatorEntry>;
	using QListEntry  = std::queue<Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList2>>;

public:

	CommandQueue() = default;

	CommandQueue(ViewPtr<ID3D12Device4> device, D3D12_COMMAND_LIST_TYPE type);
	
	CommandQueue(const CommandQueue&) = delete;
	CommandQueue& operator=(const CommandQueue&) = delete;

	CommandQueue(CommandQueue&&) = default;
	CommandQueue& operator=(CommandQueue&&) = default;

	// the destructor is not responsible for making sure if there is anything in execution in the background. the application must manually stall
	~CommandQueue() = default;

	// signals my fence
	UINT64 signal();

	// stalls the CPU if the current fence completed value has not reached value
	void wait_CPU(UINT64 value);

	// stalls this queue ( GPU ) if another queue has not finished work ( reached value )
	void wait_GPU(ViewPtr<ID3D12Fence> fence, UINT64 value);

	// signals the fence then stalls the CPU until completion
	void flush_execution();

	// execute a command list. retruns the fence value to wait for
	UINT64 execute( CommandList list );

	ID3D12CommandQueue* get_queue() const noexcept;
	ID3D12Fence*        get_fence() const noexcept;


	// get the command list
	CommandList acquire_command_list();

protected:

	Microsoft::WRL::ComPtr<ID3D12CommandAllocator> create_command_allocator() const;
	Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList2> create_command_list2() const;

private:

	D3D12_COMMAND_LIST_TYPE                    mType;
	ViewPtr<ID3D12Device4>                     mDevice       = nullptr;
	Microsoft::WRL::ComPtr<ID3D12CommandQueue> mCommandQueue = nullptr;
	
	Fence                               mFence;
	UINT64                              mFenceValue = 0ull;

	QAllocEntry                        mAllocatorQueue;
	QListEntry                         mListQueue;
};