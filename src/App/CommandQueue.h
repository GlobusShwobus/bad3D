#pragma once

#include <queue>

#include <d3d12.h>
#include <wrl/client.h>

#include "D3D12/Fence.h"
#include "Tools/ViewPtr.h"
#include "Tools/UniqueHandle.h"

#include "CommandList.h"

// TODO: thread safety review
// TODO: add execute_lists ( maybe execute(commandlists* lists, size_t size))

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

	CommandQueue() = delete;
	CommandQueue(ViewPtr<ID3D12Device4> device, D3D12_COMMAND_LIST_TYPE type);
	~CommandQueue() noexcept;

	CommandQueue(const CommandQueue&) = delete;
	CommandQueue& operator=(const CommandQueue&) = delete;
	CommandQueue(CommandQueue&&) noexcept = delete;
	CommandQueue& operator=(CommandQueue&&) noexcept = delete;

	UINT64 execute( CommandList&& list );

	UINT64 get_completed_value();
	void wait_until_completion(UINT64 until, DWORD milliseconds = INFINITE);
	void flush();

	CommandList acquire_command_list();
	ID3D12CommandQueue* get_queue() const noexcept;

protected:

	UINT64 signal();
	
	Microsoft::WRL::ComPtr<ID3D12CommandAllocator> create_command_allocator() const;
	Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList2> create_command_list2() const;

private:

	D3D12_COMMAND_LIST_TYPE mType;
	Microsoft::WRL::ComPtr<ID3D12CommandQueue> mCommandQueue;

	Microsoft::WRL::ComPtr<ID3D12Fence> mFence;
	UniqueHandle mEventHandle;
	UINT64 mFenceValue;

	QAllocEntry mAllocatorQueue;
	QListEntry mListQueue;

	ViewPtr<ID3D12Device4> mDevice;
};