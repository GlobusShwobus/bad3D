#pragma once

#include "badDirectX.h"
#include <wrl/client.h>
#include "CommandList.h"
#include "Fence.h"

#include "ViewPtr.h"
#include <queue>

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

	CommandQueue(ViewPtr<ID3D12Device4> device, D3D12_COMMAND_LIST_TYPE type);
	
	CommandQueue(const CommandQueue&) = delete;
	CommandQueue& operator=(const CommandQueue&) = delete;
	CommandQueue(CommandQueue&&) = delete;
	CommandQueue& operator=(CommandQueue&&) = delete;

	// the destructor is not responsible for making sure if there is anything in execution in the background. the application must manually stall
	virtual ~CommandQueue() = default;

	// execute a command list. retruns the fence value to wait for
	UINT64 execute( CommandList list );

	// forces a CPU stall
	void flush();

	// checks if expected value is more than fence value. if fence value is less, it will stall the CPU, otherwise nothing
	void wait(UINT64 expected_value);

	// get the command list
	CommandList acquire_command_list();
	constexpr ViewPtr<ID3D12CommandQueue> get_queue()const noexcept { return mCommandQueue.Get(); }

protected:

	UINT64 signal();

	Microsoft::WRL::ComPtr<ID3D12CommandAllocator> create_command_allocator() const;
	Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList2> create_command_list2() const;

private:

	const D3D12_COMMAND_LIST_TYPE              mType;
	ViewPtr<ID3D12Device4>                     mDevice       = nullptr;
	Microsoft::WRL::ComPtr<ID3D12CommandQueue> mCommandQueue = nullptr;
	
	Fence mFence;

	QAllocEntry                        mAllocatorQueue;
	QListEntry                         mListQueue;
};