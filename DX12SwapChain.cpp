#include "DX12SwapChain.h"
#include "Utils.h"

DX12SwapChain::DX12SwapChain(
	ObserverPtr<IDXGIFactory4> factory,
	ObserverPtr<ID3D12Device2> device,
	ObserverPtr<ID3D12CommandQueue> command_queue,
	ObserverPtr<HWND__> client_window,
	UINT64 number_of_buffers
)
	:mDevice(device), mBufferCount(number_of_buffers)
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
	swap_chain_desc.BufferCount = number_of_buffers;
	swap_chain_desc.Scaling = DXGI_SCALING_STRETCH;
	swap_chain_desc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
	swap_chain_desc.AlphaMode = DXGI_ALPHA_MODE_UNSPECIFIED;
	swap_chain_desc.Flags = mIsTearingSupported ? DXGI_SWAP_CHAIN_FLAG_ALLOW_TEARING : 0;

	// descriptor heap description
	D3D12_DESCRIPTOR_HEAP_DESC descriptor_heap_desc = {};
	descriptor_heap_desc.NumDescriptors = number_of_buffers;
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

	// create the descriptor heap
	mDescHeap.initialise(device, descriptor_heap_desc);

	// set descriptions
	reset_description_info();
}

void DX12SwapChain::present()
{
	// determine sync interval and flags
	UINT syncInterval = mIsVSync ? 1 : 0;
	UINT presentFlags = (mIsTearingSupported && !mIsVSync) ? DXGI_PRESENT_ALLOW_TEARING : 0;
	// present the current buffer. swap chain will internally change the current writable buffer index
	execute_test_throw(
		mSwapChain->Present(syncInterval, presentFlags)
	);

	// set the the current buffer views signal value, to be used as a determining if stalling CPU is required
	// mBufferViews[mCurrentBackBufferIndex].signal_value = signal_value;

	// reset the current back buffer index
	mCurrentBackBufferIndex = mSwapChain->GetCurrentBackBufferIndex();
}

void DX12SwapChain::resize_back_buffers(uint32_t width, uint32_t height)
{
	// release any references to the back buffers and set the signal value to the latest signal value
	//	const UINT64 latest_signal_value = mBufferViews[mCurrentBackBufferIndex].signal_value;
	//	for (auto& buffer : mBufferViews)
	//	{
	//		buffer.buffer.Reset();
	//		buffer.signal_value = latest_signal_value;
	//	}

	// reset swap chains back buffers
	DXGI_SWAP_CHAIN_DESC scDesc = {};
	execute_test_throw(
		mSwapChain->GetDesc(&scDesc)
	);
	execute_test_throw(
		mSwapChain->ResizeBuffers(
			mBufferCount,
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
	reset_description_info();
}

//this function maps / remaps backbuffers and descriptors. use this after resize or in anycase the buffers chang
void DX12SwapChain::reset_description_info()
{
	D3D12_CPU_DESCRIPTOR_HANDLE heapPos = mDescHeap.get_descriptor_handle_for(NULL);

	for (UINT i = 0; i < mBufferCount; i++)
	{
		//auto& buffer = mBufferViews[i].buffer;
		//// release current handle
		//buffer.Reset();
		//// get pointer to the resource
		//mSwapChain->GetBuffer(i, IID_PPV_ARGS(&buffer));
		//// assign type metadata on top of current data
		mDevice->CreateRenderTargetView(get_buffer_at(i).Get(), nullptr, heapPos);
		//increment pos
		heapPos.ptr += mDescHeap.desc_size();
	}
}