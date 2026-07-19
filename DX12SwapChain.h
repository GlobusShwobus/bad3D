#pragma once

#include "GPU_CORE.h"
#include "Utils.h"
#include "ObserverPtr.h"

class DX12SwapChain
{
	static constexpr int NUMBER_OF_BUFFERS = 3;

public:
	DX12SwapChain(
		ObserverPtr<IDXGIFactory4> factory,
		ObserverPtr<ID3D12Device2> device,
		ObserverPtr<ID3D12CommandQueue> command_queue,
		HWND client_window,
		uint32_t client_width, 
		uint32_t client_height)
		:mDevice(device), mWidth(client_width), mHeight(client_height)
	{
		// check if tearing is supported
		mIsTearingSupported = check_is_tearing_supported(factory);
		// temp just set vsync to true
		mIsVSync = true;

		// swap chain description
		DXGI_SWAP_CHAIN_DESC1 swap_chain_desc = {};
		swap_chain_desc.Width        = client_width;
		swap_chain_desc.Height       = client_height;
		swap_chain_desc.Format       = DXGI_FORMAT_R8G8B8A8_UNORM;
		swap_chain_desc.Stereo       = FALSE;
		swap_chain_desc.SampleDesc   = { 1,0 };
		swap_chain_desc.BufferUsage  = DXGI_USAGE_RENDER_TARGET_OUTPUT;
		swap_chain_desc.BufferCount  = NUMBER_OF_BUFFERS;
		swap_chain_desc.Scaling      = DXGI_SCALING_STRETCH;
		swap_chain_desc.SwapEffect   = DXGI_SWAP_EFFECT_FLIP_DISCARD;
		swap_chain_desc.AlphaMode    = DXGI_ALPHA_MODE_UNSPECIFIED;
		swap_chain_desc.Flags        = mIsTearingSupported ? DXGI_SWAP_CHAIN_FLAG_ALLOW_TEARING : 0;

		// descriptor heap description
		D3D12_DESCRIPTOR_HEAP_DESC descriptor_heap_desc = {};
		descriptor_heap_desc.NumDescriptors = NUMBER_OF_BUFFERS;
		descriptor_heap_desc.Type           = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
		descriptor_heap_desc.NodeMask       = 0;
		descriptor_heap_desc.Flags          = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;

		// Disable the Alt+Enter fullscreen toggle feature when handling DX stuff
		execute_test_throw(
			factory->MakeWindowAssociation(client_window, DXGI_MWA_NO_ALT_ENTER)
		);

		// try to create the swapchain1 then query interface it into swapchain4
		DXSwapChain1 swapchain1;
		execute_test_throw(
			factory->CreateSwapChainForHwnd(
				command_queue.get(),
				client_window,
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
		reset_back_buffers();
	}

	DX12SwapChain(const DX12SwapChain&) = delete;
	DX12SwapChain& operator= (const DX12SwapChain&) = delete;
	DX12SwapChain(DX12SwapChain&&) = delete;
	DX12SwapChain& operator=(DX12SwapChain&&) = delete;
	virtual ~DX12SwapChain() = default;

	void resize_back_buffers(uint32_t width, uint32_t height)
	{
		mWidth = width;
		mHeight = height;
			// any references to the back buffers must be released
			// for proper bookkeeping the expected fence values are also set to whatever is the most current one 
			// because old buffers are gone therefor nothing should be expected regarding them. it's essentially a reset value, nothing more.
		for (int i = 0; i < NUMBER_OF_BUFFERS; i++)
		{
			mBackBuffers[i].Reset();
			// mExpected_fence_values[i] = mExpected_fence_values[mCurrent_back_buffer_index];
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
		// reset current index
		mCurrentBackBufferIndex = mSwapChain->GetCurrentBackBufferIndex();
		// reset back buffers and descriptions
		reset_back_buffers();
	}

	// this function maps / remaps backbuffers and descriptors. use this after resize or in anycase the buffers chang
	void reset_back_buffers()
	{
		D3D12_CPU_DESCRIPTOR_HANDLE heapPos(mDescriptorHeap->GetCPUDescriptorHandleForHeapStart());

		for (UINT i = 0; i < NUMBER_OF_BUFFERS; i++)
		{
			// release current handle
			mBackBuffers[i].Reset();
			// reassign handle
			mSwapChain->GetBuffer(i, IID_PPV_ARGS(&mBackBuffers[i]));
			// rewrite description in heap
			mDevice->CreateRenderTargetView(mBackBuffers[i].Get(), nullptr, heapPos);
			//increment pos
			heapPos.ptr += mDescriptorSize;
		}
	}

	HRESULT present()
	{
		UINT syncInterval = mIsVSync ? 1 : 0;
		UINT presentFlags = (mIsTearingSupported && !mIsVSync) ? DXGI_PRESENT_ALLOW_TEARING : 0;

		HRESULT hr = mSwapChain->Present(syncInterval, presentFlags);

		mCurrentBackBufferIndex = mSwapChain->GetCurrentBackBufferIndex();
		return hr;
	}
	D3D12_RESOURCE_BARRIER barrier_from_present_to_rtv()
	{
		ObserverPtr<ID3D12Resource> back_buffer = mBackBuffers[mCurrentBackBufferIndex].Get();

		D3D12_RESOURCE_BARRIER transition_rt = {};
		transition_rt.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
		transition_rt.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
		transition_rt.Transition.pResource = back_buffer.get();
		transition_rt.Transition.StateBefore = D3D12_RESOURCE_STATE_PRESENT;         // this knowledge is implied and for back buffers it's fine because they're only ever in one or the other state. for other resources that get more complex logic, state tracking becomes important
		transition_rt.Transition.StateAfter = D3D12_RESOURCE_STATE_RENDER_TARGET;
		transition_rt.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;

		return transition_rt;
	}

	D3D12_RESOURCE_BARRIER barrier_from_rtv_to_present()
	{
		ObserverPtr<ID3D12Resource> back_buffer = mBackBuffers[mCurrentBackBufferIndex].Get();

		D3D12_RESOURCE_BARRIER transition_rt = {};
		transition_rt.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
		transition_rt.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
		transition_rt.Transition.pResource = back_buffer.get();
		transition_rt.Transition.StateBefore = D3D12_RESOURCE_STATE_RENDER_TARGET;
		transition_rt.Transition.StateAfter = D3D12_RESOURCE_STATE_PRESENT;
		transition_rt.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;

		return transition_rt;
	}

	// gets the current handle ( behaves like an index actually ) of the descriptor
	D3D12_CPU_DESCRIPTOR_HANDLE get_descriptor_index() const
	{
		D3D12_CPU_DESCRIPTOR_HANDLE index = { 0 };
		index.ptr += mDescriptorHeap->GetCPUDescriptorHandleForHeapStart().ptr + (mCurrentBackBufferIndex * mDescriptorSize);
		return index;
	}

	LONG get_width()const { return mWidth; }
	LONG get_height()const { return mHeight; }

private:

	ObserverPtr<ID3D12Device2> mDevice = nullptr;
	DXSwapChain4       mSwapChain;
	DXDescriptorHeap   mDescriptorHeap;
	DXResource         mBackBuffers[NUMBER_OF_BUFFERS];

	UINT               mDescriptorSize;
	UINT               mCurrentBackBufferIndex;

	LONG mWidth;
	LONG mHeight;

	bool mIsVSync;
	bool mIsTearingSupported;

};
