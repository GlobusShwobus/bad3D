#include "App/CommandQueue.h"

#include <assert.h>

#include <utility>
#include <stdexcept>

#include "D3D12/EasyDirectXUtils.h"

CommandQueue::CommandQueue(ViewPtr<ID3D12Device4> device, D3D12_COMMAND_LIST_TYPE type)
	:mFenceValue(0ull), mType(type), mDevice(device)
{
	if (mType != D3D12_COMMAND_LIST_TYPE_DIRECT && mType != D3D12_COMMAND_LIST_TYPE_COMPUTE && mType != D3D12_COMMAND_LIST_TYPE_COPY)
		throw std::runtime_error{"invalid command list type"};

	if (!mDevice)
		throw std::runtime_error{ "device is nullptr" };

	// create command queue for graphics
	D3D12_COMMAND_QUEUE_DESC command_queue_desc = {
		mType,
		D3D12_COMMAND_QUEUE_PRIORITY_NORMAL,
		D3D12_COMMAND_QUEUE_FLAG_NONE,
		0
	};
	
	execute_and_test_hresult(
		mDevice->CreateCommandQueue(&command_queue_desc, IID_PPV_ARGS(&mCommandQueue))
	);

	execute_and_test_hresult(
		mDevice->CreateFence(mFenceValue, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&mFence))
	);

	mEventHandle.reset(::CreateEventW(nullptr, FALSE, FALSE, nullptr));
	if (!mEventHandle)
		throw_error_code_translation( GetLastError() );
}

CommandQueue::~CommandQueue() noexcept
{
	if (mCommandQueue && mFence)
		flush();
}

UINT64 CommandQueue::execute( CommandList&& list )
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

UINT64 CommandQueue::get_completed_value()
{
	return mFence->GetCompletedValue();
}

void CommandQueue::wait_until_completion(UINT64 until, DWORD milliseconds)
{
	if (get_completed_value() >= until)
		return;

	execute_and_test_hresult(
		mFence->SetEventOnCompletion(until, mEventHandle.get())
	);

	::WaitForSingleObject(mEventHandle.get(), milliseconds);
}

UINT64 CommandQueue::signal()
{
	const UINT64 value = mFenceValue++;

	mCommandQueue->Signal(mFence.Get(), value);

	return value;
}

void CommandQueue::flush()
{
	wait_until_completion(
		signal(), INFINITE
	);
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

CommandList CommandQueue::acquire_command_list()
{
	CommandList context;

	// if there is at least allocator in the queue attempt to reuse it.
	// ID3D2CommandQueue internal signal counter always increments sequentially.
	// Since the queue is first in first out it works out generally but the second check for value is still required for safety.
	if (!mAllocatorQueue.empty() && (get_completed_value() >= mAllocatorQueue.front().fence_value))
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

ID3D12CommandQueue* CommandQueue::get_queue() const noexcept
{
	return mCommandQueue.Get();
}

