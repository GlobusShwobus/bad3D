#include "DX12CommandQueue.h"
#include "Utils.h"

DX12CommandQueue::DX12CommandQueue(ObserverPtr<ID3D12Device4> device, D3D12_COMMAND_LIST_TYPE type)
	:mType(type), mDevice(device), mFence(device)
{
	// create command queue for graphics
	D3D12_COMMAND_QUEUE_DESC command_queue_desc = {};
	command_queue_desc.Type = type;            // command list type and command queue types must match. generally either: direct, compute or copy but there are others 
	command_queue_desc.Priority = D3D12_COMMAND_QUEUE_PRIORITY_NORMAL;   // for rendering normal, for non sequential use high priority ( not sure for what currently )
	command_queue_desc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;            // enable / disable GPU timeouts. keep default enabled
	command_queue_desc.NodeMask = 0;
	
	if(!device) // fence will throw before this check though...
		throw_error_code_translation(static_cast<DWORD>(E_POINTER));

	execute_and_test_hresult(
		mDevice->CreateCommandQueue(&command_queue_desc, IID_PPV_ARGS(&mCommandQueue))
	);

	execute_and_test_hresult(
		mDevice->CreateCommandList1(NULL, mType, D3D12_COMMAND_LIST_FLAG_NONE, IID_PPV_ARGS(&mCommandContext.command_list))
	);
}

UINT64 DX12CommandQueue::execute()
{
	// because command queue wants lists not a list
	ID3D12CommandList* const command_lists[] = { mCommandContext.command_list.Get() };

	// execute list(s)
	mCommandQueue->ExecuteCommandLists(_countof(command_lists), command_lists);

	// signal command queue
	UINT64 signal_value = signal();

	// store the allocator and list. MOVE command allocator, don't copy. avoiding magic ref count
	mAllocatorQueue.emplace(CommandAllocatorQueueEntry{ signal_value , std::move(mCommandContext.command_allocator)});

	return signal_value;
}

void DX12CommandQueue::flush()
{
	wait( signal() );
}

UINT64 DX12CommandQueue::signal()
{
	// get the current counter value which will be used to signal the GPU
	UINT64 signaled_value = mFence.get_counter_value();
	// increment the counter for the next call
	mFence.increment_counter();

	// signal the GPU with the value and fence
	execute_and_test_hresult(
		mCommandQueue->Signal(mFence.get_observer().get(), signaled_value)
	);

	return signaled_value;
}

// checks if the internal fence value has reached the expected value. if not stalls the CPU
void DX12CommandQueue::wait(UINT64 expected_value)
{
	// if current completed value is less than expected value, then stalling is required
	if (mFence.get_completed_value() < expected_value)
	{
		mFence.stall_thread_until(expected_value);
	}
}

ObserverPtr<ID3D12GraphicsCommandList2> DX12CommandQueue::get_command_list()
{
	// if there is at least allocator in the queue attempt to reuse it.
	// ID3D2CommandQueue internal signal counter always increments sequentially.
	// Since the queue is first in first out it works out generally but the second check for value is still required for safety.
	if (!mAllocatorQueue.empty() && (mFence.get_completed_value() >= mAllocatorQueue.front().fence_value))
	{
		mCommandContext.command_allocator = mAllocatorQueue.front().command_allocator;
		mAllocatorQueue.pop();

		execute_and_test_hresult(
			mCommandContext.command_allocator->Reset() // indicates to re-use memory, not to let go of the ptr
		);
	}
	else // otherwise create a new allocator
	{
		mCommandContext.command_allocator = create_command_allocator();
	}

	// reset the command list and tie it with the new allocator
	execute_and_test_hresult(
		mCommandContext.command_list->Reset(mCommandContext.command_allocator.Get(), nullptr)
	);

	return mCommandContext.command_list.Get();
}

D3D12CommandAllocator DX12CommandQueue::create_command_allocator()
{
	D3D12CommandAllocator allocator;

	execute_and_test_hresult(
		mDevice->CreateCommandAllocator(mType, IID_PPV_ARGS(&allocator))
	);

	return allocator;
}