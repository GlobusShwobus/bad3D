#include "App/CommandQueue.h"

#include <utility>
#include <stdexcept>

#include "D3D12/EasyDirectXUtils.h"
#include "D3D12/CommandList.h"
#include "Tools/UniqueHandle.h"

CommandQueue::CommandQueue(ID3D12Device4* device, D3D12_COMMAND_LIST_TYPE type)
	: mType(type), mDevice(device), mFenceValue(0ull)
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
}

CommandQueue::~CommandQueue()
{
	if (mCommandQueue && mFence)
		flush();
}

void CommandQueue::submit(std::unique_ptr<CommandList> list)
{
	if (!list)
		throw std::invalid_argument("nullptr list not allowed");

	execute_and_test_hresult(
		list->command_list->Close()
	);

	std::lock_guard lock(mLock);
	mSubmitList.push_back(std::move(list));
}

UINT64 CommandQueue::execute( )
{
	std::lock_guard lock(mLock);

	// if nothing is pending, early out
	if (mSubmitList.empty())
		return mFenceValue;

	// create a contiguous array command lists
	std::vector<ID3D12CommandList*> execution_list;
	execution_list.reserve(mSubmitList.size());

	for (const auto& l : mSubmitList)
		execution_list.push_back(l->command_list.Get());

	mCommandQueue->ExecuteCommandLists(static_cast<UINT>(execution_list.size()), execution_list.data());

	const UINT64 fence_value = signal();

	for (auto& l : mSubmitList)
		mFlightQueue.push(CommandListEntry{ fence_value, std::move(l) });

	mSubmitList.clear();

	return fence_value;
}

bool CommandQueue::wait_until_completion(UINT64 until, DWORD milliseconds)
{
	if (mFence->GetCompletedValue() >= until)
		return true;

	UniqueHandle event_handle(::CreateEventW(nullptr, FALSE, FALSE, nullptr));

	if (!event_handle)
		throw std::runtime_error("Failed to create fence event handle");

	execute_and_test_hresult(
		mFence->SetEventOnCompletion(until, event_handle.get())
	);

	return ::WaitForSingleObject(event_handle.get(), milliseconds) == WAIT_OBJECT_0;
}

UINT64 CommandQueue::signal()
{
	const UINT64 value = ++mFenceValue;

	execute_and_test_hresult(
		mCommandQueue->Signal(mFence.Get(), value)
	);

	return value;
}

void CommandQueue::flush()
{
	UINT64 value;
	{
		std::lock_guard lock(mLock);
		value = signal();
	}
	wait_until_completion( value, INFINITE );
}

std::unique_ptr<CommandList> CommandQueue::get_command_list()
{
	std::unique_ptr<CommandList> list;

	{
		// only lock the pop. the list is local so after this case it's thread safe
		std::lock_guard lock(mLock);
		if (!mFlightQueue.empty() && mFence->GetCompletedValue() >= mFlightQueue.front().fence_value)
		{
			list = std::move(mFlightQueue.front().list);
			mFlightQueue.pop();
		}
	}

	if (list)
	{
		execute_and_test_hresult(list->command_allocator->Reset());
	}
	else
	{
		list = std::make_unique<CommandList>(
			create_command_allocator(mDevice.get(), mType),
			create_command_list2(mDevice.get(), mType, D3D12_COMMAND_LIST_FLAG_NONE)
		);
	}

	execute_and_test_hresult(
		list->command_list->Reset(list->command_allocator.Get(), nullptr)
	);

	return list;
}
