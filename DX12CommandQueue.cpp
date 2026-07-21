#include "DX12CommandQueue.h"
#include "Utils.h"


DX12CommandQueue::DX12CommandQueue(ObserverPtr<ID3D12Device2> device, D3D12_COMMAND_LIST_TYPE type)
	:mDevice(device), mType(type), mFenceSignalCounter(0)
{
	D3D12_COMMAND_QUEUE_DESC desc = {};
	desc.Type = mType;                                // command queue and command list must be the same type
	desc.Priority = D3D12_COMMAND_QUEUE_PRIORITY_NORMAL;  // don't think we're doing anything special which requires priviledge
	desc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;        // only 2 modes, the other being disabling timeouts
	desc.NodeMask = 0;                                    // for multi adapter systems

	execute_test_throw(
		mDevice->CreateCommandQueue(&desc, IID_PPV_ARGS(&mCommandQueue))
	);

	execute_test_throw(
		device->CreateFence(0ull, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&mFence))
	);

	mFenceEventHandle = ::CreateEventW(NULL, FALSE, FALSE, NULL);
	if (!mFenceEventHandle)
		throw_error_code_translation(GetLastError());
}

DX12CommandQueue::~DX12CommandQueue()
{
	flush();

	if (mFenceEventHandle)
		::CloseHandle(mFenceEventHandle);
}

D3D12GraphicsCommandList2 DX12CommandQueue::get_command_list()
{
	D3D12CommandAllocator command_allocator;
	D3D12GraphicsCommandList2 command_list;
	const UINT64 current_fecnce_value = mFence->GetCompletedValue();

	if (!mCommandAllocatorQueue.empty() && is_fence_complete(mCommandAllocatorQueue.front().fence_value))
	{
		command_allocator = mCommandAllocatorQueue.front().command_allocator;
		mCommandAllocatorQueue.pop();

		execute_test_throw(
			command_allocator->Reset()
		);
	}
	else
	{
		command_allocator = create_command_allocator();
	}

	if (!mCommandListQueue.empty())
	{
		command_list = mCommandListQueue.front();
		mCommandListQueue.pop();

		execute_test_throw(
			command_list->Reset(command_allocator.Get(), nullptr)
		);
	}
	else
	{
		command_list = create_command_list(command_allocator);
	}

	// Associate the command allocator with the command list so that it can be
	// retrieved when the command list is executed.
	execute_test_throw(
		command_list->SetPrivateDataInterface(__uuidof(ID3D12CommandAllocator), command_allocator.Get())
	);

	return command_list;
}

UINT64 DX12CommandQueue::execute(D3D12GraphicsCommandList2 command_list)
{
	// get associated command allocator
	ID3D12CommandAllocator* command_allocator;
	UINT data_size = sizeof(command_allocator);

	execute_test_throw(
		command_list->GetPrivateData(__uuidof(ID3D12CommandAllocator), &data_size, &command_allocator)
	);

	// eventually it will be cool to have multiple list. command queue wants an array of lists by defualt. right now it's just 1 though
	ID3D12CommandList* const command_lists[] = { command_list.Get() };

	// execute list(s)
	mCommandQueue->ExecuteCommandLists(_countof(command_lists), command_lists);

	// signal command queue
	UINT64 signal_value = signal();

	// store the allocator and list
	mCommandAllocatorQueue.emplace(CommandAllocatorEntry{ signal_value , command_allocator });
	mCommandListQueue.push(command_list);

	// The ownership of the command allocator has been transferred to the ComPtr
	// in the command allocator queue. It is safe to release the reference 
	// in this temporary COM pointer here.
	command_allocator->Release();

	return signal_value;
}

void DX12CommandQueue::flush()
{
	wait(signal());
}

ID3D12CommandQueue* DX12CommandQueue::get_private()const
{
	return mCommandQueue.Get();
}

UINT64 DX12CommandQueue::signal()
{
	UINT64 signaled_value = ++mFenceSignalCounter;

	execute_test_throw(
		mCommandQueue->Signal(mFence.Get(), signaled_value)
	);

	return signaled_value;
}

// checks if the internal fence value has reached the expected value. if not stalls the CPU
void DX12CommandQueue::wait(UINT64 expected_value)
{
	// check if waiting is required at all
	if (!is_fence_complete(expected_value))
	{
		// trigger event when value is reached
		execute_test_throw(
			mFence->SetEventOnCompletion(expected_value, mFenceEventHandle)
		);
		// stall the CPU thread
		::WaitForSingleObject(mFenceEventHandle, 0xFFFFFFFF);
	}
}

bool DX12CommandQueue::is_fence_complete(UINT64 value)const
{
	return mFence->GetCompletedValue() >= value;
}

D3D12CommandAllocator DX12CommandQueue::create_command_allocator()
{
	D3D12CommandAllocator dxca;

	execute_test_throw(
		mDevice->CreateCommandAllocator(mType, IID_PPV_ARGS(&dxca))
	);

	return dxca;
}

D3D12GraphicsCommandList2 DX12CommandQueue::create_command_list(D3D12CommandAllocator command_allocator)
{
	D3D12GraphicsCommandList2 dxcl2;
	execute_test_throw(
		mDevice->CreateCommandList(NULL, mType, command_allocator.Get(), nullptr, IID_PPV_ARGS(&dxcl2))
	);

	//	execute_test_throw(
	//		dxcl2->Close()
	//	);

	return dxcl2;
}