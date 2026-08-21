#include "CommandQueue.h"
#include "Utils.h"

CommandQueue::CommandQueue(ViewPtr<ID3D12Device4> device, D3D12_COMMAND_LIST_TYPE type)
	:mType(type), mFence(device)
{
	assert(device && "device nullptr");

	// create command queue for graphics
	D3D12_COMMAND_QUEUE_DESC command_queue_desc = {};
	command_queue_desc.Type = type;            // command list type and command queue types must match. generally either: direct, compute or copy but there are others 
	command_queue_desc.Priority = D3D12_COMMAND_QUEUE_PRIORITY_NORMAL;   // for rendering normal, for non sequential use high priority ( not sure for what currently )
	command_queue_desc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;            // enable / disable GPU timeouts. keep default enabled
	command_queue_desc.NodeMask = 0;
	
	execute_and_test_hresult(
		device->CreateCommandQueue(&command_queue_desc, IID_PPV_ARGS(&mCommandQueue))
	);

	mDevice = device;
}

UINT64 CommandQueue::execute( CommandList list )
{
	// close the list
	list.command_list->Close();

	// because command queue wants lists not a list
	ID3D12CommandList* const command_lists[] = { list.command_list.Get() };

	// execute list(s)
	mCommandQueue->ExecuteCommandLists(_countof(command_lists), command_lists);

	// signal command queue
	UINT64 signal_value = signal();

	// store the allocator and list. MOVE command allocator, don't copy. avoiding magic ref count
	mAllocatorQueue.emplace(
		CommandAllocatorEntry{ signal_value , std::move(list.command_allocator)}
	);

	mListQueue.push( std::move(list.command_list));

	return signal_value;
}

void CommandQueue::flush()
{
	wait( signal() );
}

UINT64 CommandQueue::signal()
{
	return mFence.signal( mCommandQueue.Get() );
}

Microsoft::WRL::ComPtr<ID3D12CommandAllocator> CommandQueue::create_command_allocator() const
{
	Microsoft::WRL::ComPtr<ID3D12CommandAllocator> allocator;

	execute_and_test_hresult(
		mDevice->CreateCommandAllocator(mType, IID_PPV_ARGS(&allocator))
	);

	return allocator;
}

Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList2> CommandQueue::create_command_list2() const
{
	Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList2> list;

	execute_and_test_hresult(
		mDevice->CreateCommandList1(NULL, mType, D3D12_COMMAND_LIST_FLAG_NONE, IID_PPV_ARGS(&list))
	);

	return list;
}

// checks if the internal fence value has reached the expected value. if not stalls the CPU
void CommandQueue::wait(UINT64 expected_value)
{
	// if current completed value is less than expected value, then stalling is required
	if (mFence.get_completed_value() < expected_value)
		mFence.wait(expected_value);
}

CommandList CommandQueue::acquire_command_list()
{
	CommandList context;

	// if there is at least allocator in the queue attempt to reuse it.
	// ID3D2CommandQueue internal signal counter always increments sequentially.
	// Since the queue is first in first out it works out generally but the second check for value is still required for safety.
	if (!mAllocatorQueue.empty() && (mFence.get_completed_value() >= mAllocatorQueue.front().fence_value))
	{
		context.command_allocator = mAllocatorQueue.front().command_allocator;
		mAllocatorQueue.pop();

		execute_and_test_hresult(
			context.command_allocator->Reset() // indicates to re-use memory, not to let go of the ptr
		);
	}
	else // otherwise create a new allocator
	{
		context.command_allocator = create_command_allocator();
	}

	if (!mListQueue.empty())
	{
		context.command_list = mListQueue.front();
		mListQueue.pop();
	}
	else
	{
		context.command_list = create_command_list2();
	}

	execute_and_test_hresult(
		context.command_list->Reset(context.command_allocator.Get(), nullptr)
	);

	return context;
}