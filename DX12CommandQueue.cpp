#include "DX12CommandQueue.h"
#include "Utils.h"

DX12CommandQueue::DX12CommandQueue(D3D12_COMMAND_QUEUE_DESC desc, ObserverPtr<ID3D12Device2> device)
	:mDevice(device), mType(desc.Type), mFence(device)
{
	if(!device) // fence will throw before this check though...
		throw_error_code_translation(static_cast<DWORD>(E_POINTER));

	execute_test_throw(
		mDevice->CreateCommandQueue(&desc, IID_PPV_ARGS(&mCommandQueue))
	);
}

DX12CommandQueue::~DX12CommandQueue()
{
	flush();
}

D3D12GraphicsCommandList2 DX12CommandQueue::get_command_list()
{
	// keep local vals ComPtr because there's not much control here. something might fail and it would leak here.
	D3D12CommandAllocator command_allocator;
	D3D12GraphicsCommandList2 command_list;

	// if there is at least allocator in the queue attempt to reuse it.
	// ID3D2CommandQueue internal signal counter always increments sequentially.
	// Since the queue is first in first out it works out generally but the second check for value is still required for safety.
	if (!mAllocatorQueue.empty() && (mFence.get_completed_value() >= mAllocatorQueue.front().fence_value))
	{
		command_allocator = mAllocatorQueue.front().command_allocator;
		mAllocatorQueue.pop();

		execute_test_throw(
			command_allocator->Reset() // indicates to re-use memory, not to let go of the ptr
		);
	}
	else // otherwise create a new allocator
	{
		command_allocator = create_command_allocator();
	}

	// Same exact logic with the list queue (however the list queue is not as vital)
	if (!mListQueue.empty())
	{
		command_list = mListQueue.front();
		mListQueue.pop();

		execute_test_throw(
			command_list->Reset(command_allocator.Get(), nullptr)
		);
	}
	else
	{
		command_list = create_command_list(command_allocator.Get());
	}

	// Associate the command allocator with the command list so that it can be retrieved when the command list is executed.
	// WARNING: associating to private data of IObject will increment ref count
	execute_test_throw(
		command_list->SetPrivateDataInterface(__uuidof(ID3D12CommandAllocator), command_allocator.Get())
	);

	return command_list;
}

UINT64 DX12CommandQueue::execute(D3D12GraphicsCommandList2 command_list)
{
	// get associated command allocator
	D3D12CommandAllocator command_allocator; // note: guide code used raw ptr but using a comptr will simply avoid all the bullshit of win32
	UINT data_size = sizeof(command_allocator);

	// WARNING: GetPrivateData will increment the ref count
	execute_test_throw(
		command_list->GetPrivateData(__uuidof(ID3D12CommandAllocator), &data_size, command_allocator.ReleaseAndGetAddressOf())
	);

	// because command queue wants lists not a list
	ID3D12CommandList* const command_lists[] = { command_list.Get() };

	// execute list(s)
	mCommandQueue->ExecuteCommandLists(_countof(command_lists), command_lists);

	// signal command queue
	UINT64 signal_value = signal();

	// store the allocator and list. MOVE command allocator, don't copy. avoiding magic ref count
	mAllocatorQueue.emplace(CommandAllocatorEntry{ signal_value , std::move(command_allocator)});
	mListQueue.push(command_list);

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
	execute_test_throw(
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

D3D12CommandAllocator DX12CommandQueue::create_command_allocator()
{
	D3D12CommandAllocator allocator;

	execute_test_throw(
		mDevice->CreateCommandAllocator(mType, IID_PPV_ARGS(&allocator))
	);

	return allocator;
}

D3D12GraphicsCommandList2 DX12CommandQueue::create_command_list(ObserverPtr<ID3D12CommandAllocator> command_allocator)
{
	D3D12GraphicsCommandList2 list;
	execute_test_throw(
		mDevice->CreateCommandList(NULL, mType, command_allocator.get(), nullptr, IID_PPV_ARGS(&list))
	);

	return list;
}