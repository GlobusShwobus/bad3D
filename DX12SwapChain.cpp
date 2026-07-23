#include "DX12SwapChain.h"
#include "Utils.h"

DX12SwapChain::DX12SwapChain(
	ObserverPtr<IDXGIFactory4> factory,
	ObserverPtr<ID3D12Device2> device,
	ObserverPtr<ID3D12CommandQueue> command_queue,
	ObserverPtr<HWND__> client_window
)
	:mDevice(device)
{
	if (!factory || !device || !command_queue || !client_window)
		throw_error_code_translation(static_cast<DWORD>(E_POINTER));

	// check if tearing is supported
	mIsTearingSupported = check_is_tearing_supported(factory);

	// temp just set vsync to true
	mIsVSync = true;

	// query client size
	query_client_size(client_window.get(), mWidth, mHeight);

	// Disable the Alt+Enter fullscreen toggle feature when handling DX stuff
	execute_test_throw(
		factory->MakeWindowAssociation(client_window.get(), DXGI_MWA_NO_ALT_ENTER)
	);

	// swap chain description
	DXGI_SWAP_CHAIN_DESC1 swap_chain_desc = {};
	swap_chain_desc.Width = mWidth;
	swap_chain_desc.Height = mHeight;
	swap_chain_desc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	swap_chain_desc.Stereo = FALSE;
	swap_chain_desc.SampleDesc = { 1,0 };
	swap_chain_desc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	swap_chain_desc.BufferCount = NUMBER_OF_BUFFERS;
	swap_chain_desc.Scaling = DXGI_SCALING_STRETCH;
	swap_chain_desc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
	swap_chain_desc.AlphaMode = DXGI_ALPHA_MODE_UNSPECIFIED;
	swap_chain_desc.Flags = mIsTearingSupported ? DXGI_SWAP_CHAIN_FLAG_ALLOW_TEARING : 0;

	// descriptor heap description
	D3D12_DESCRIPTOR_HEAP_DESC descriptor_heap_desc = {};
	descriptor_heap_desc.NumDescriptors = NUMBER_OF_BUFFERS;
	descriptor_heap_desc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
	descriptor_heap_desc.NodeMask = 0;
	descriptor_heap_desc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;

	// try to create the swapchain1 then query interface it into swapchain4
	DXGISwapChain1 swapchain1;
	execute_test_throw(
		factory->CreateSwapChainForHwnd(
			command_queue.get(),
			client_window.get(),
			&swap_chain_desc,
			nullptr,
			nullptr,
			&swapchain1
		)
	);
	execute_test_throw(
		swapchain1.As(&mSwapChain)
	);

	// the swap chain manages it's internal buffers itself. store the real current index
	mCurrentBackBufferIndex = mSwapChain->GetCurrentBackBufferIndex();

	// store the size of a single descriptor for this system
	mDescriptorSize = mDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);

	// create the descriptor heap
	execute_test_throw(
		mDevice->CreateDescriptorHeap(&descriptor_heap_desc, IID_PPV_ARGS(&mDescriptorHeap))
	);

	// set back buffers
	update_back_buffers_views();
}

void DX12SwapChain::present(UINT64 signal_value)
{
	// determine sync interval and flags
	UINT syncInterval = mIsVSync ? 1 : 0;
	UINT presentFlags = (mIsTearingSupported && !mIsVSync) ? DXGI_PRESENT_ALLOW_TEARING : 0;
	// present the current buffer. swap chain will internally change the current writable buffer index
	execute_test_throw(
		mSwapChain->Present(syncInterval, presentFlags)
	);

	// set the the current buffer views signal value, to be used as a determining if stalling CPU is required
	mBufferViews[mCurrentBackBufferIndex].signal_value = signal_value;

	// reset the current back buffer index
	mCurrentBackBufferIndex = mSwapChain->GetCurrentBackBufferIndex();
}

void DX12SwapChain::resize_back_buffers(uint32_t width, uint32_t height)
{
	// release any references to the back buffers and set the signal value to the latest signal value
	const UINT64 latest_signal_value = mBufferViews[mCurrentBackBufferIndex].signal_value;
	for (auto& buffer : mBufferViews)
	{
		buffer.buffer.Reset();
		buffer.signal_value = latest_signal_value;
	}

	// reset swap chains back buffers
	DXGI_SWAP_CHAIN_DESC scDesc = {};
	execute_test_throw(
		mSwapChain->GetDesc(&scDesc)
	);
	execute_test_throw(
		mSwapChain->ResizeBuffers(
			NUMBER_OF_BUFFERS,
			width,
			height,
			scDesc.BufferDesc.Format,
			scDesc.Flags
		));

	// set size handles
	mWidth = width;
	mHeight = height;

	// reset current index
	mCurrentBackBufferIndex = mSwapChain->GetCurrentBackBufferIndex();

	// update back buffer handles
	update_back_buffers_views();
}

// this function maps / remaps backbuffers and descriptors. use this after resize or in anycase the buffers chang
void DX12SwapChain::update_back_buffers_views()
{
	D3D12_CPU_DESCRIPTOR_HANDLE heapPos(mDescriptorHeap->GetCPUDescriptorHandleForHeapStart());

	for (UINT i = 0; i < NUMBER_OF_BUFFERS; i++)
	{
		auto& buffer = mBufferViews[i].buffer;
		// release current handle
		buffer.Reset();
		// get pointer to the resource
		mSwapChain->GetBuffer(i, IID_PPV_ARGS(&buffer));
		// assign type metadata on top of current data
		mDevice->CreateRenderTargetView(buffer.Get(), nullptr, heapPos);
		//increment pos
		heapPos.ptr += mDescriptorSize;
	}
}

D3D12_RESOURCE_BARRIER DX12SwapChain::command_from_present_to_rtv() const
{
	ObserverPtr<ID3D12Resource> back_buffer = mBufferViews[mCurrentBackBufferIndex].buffer.Get();

	D3D12_RESOURCE_BARRIER transition_rt = {};
	transition_rt.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
	transition_rt.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
	transition_rt.Transition.pResource = back_buffer.get();
	transition_rt.Transition.StateBefore = D3D12_RESOURCE_STATE_PRESENT;         // this knowledge is implied and for back buffers it's fine because they're only ever in one or the other state. for other resources that get more complex logic, state tracking becomes important
	transition_rt.Transition.StateAfter = D3D12_RESOURCE_STATE_RENDER_TARGET;
	transition_rt.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;

	return transition_rt;
}

D3D12_RESOURCE_BARRIER DX12SwapChain::command_from_rtv_to_present() const
{
	ObserverPtr<ID3D12Resource> back_buffer = mBufferViews[mCurrentBackBufferIndex].buffer.Get();

	D3D12_RESOURCE_BARRIER transition_rt = {};
	transition_rt.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
	transition_rt.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
	transition_rt.Transition.pResource = back_buffer.get();
	transition_rt.Transition.StateBefore = D3D12_RESOURCE_STATE_RENDER_TARGET;
	transition_rt.Transition.StateAfter = D3D12_RESOURCE_STATE_PRESENT;
	transition_rt.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;

	return transition_rt;
}

D3D12_CPU_DESCRIPTOR_HANDLE DX12SwapChain::command_backbuffer_handle() const
{
	D3D12_CPU_DESCRIPTOR_HANDLE handle = { 0 };
	handle.ptr += mDescriptorHeap->GetCPUDescriptorHandleForHeapStart().ptr + (mCurrentBackBufferIndex * mDescriptorSize);
	return handle;
}