#pragma once

#include "GPU_CORE.h"
#include "ObserverPtr.h"
#include "DX12DescriptorHeap.h"
#include <array>

class DX12SwapChain final
{
public:
	DX12SwapChain(
		ObserverPtr<IDXGIFactory4> factory,
		ObserverPtr<ID3D12Device2> device,
		ObserverPtr<ID3D12CommandQueue> command_queue,
		ObserverPtr<HWND__> client_window,
		UINT64 number_of_buffers
	);

	DX12SwapChain(const DX12SwapChain&) = delete;
	DX12SwapChain& operator= (const DX12SwapChain&) = delete;
	DX12SwapChain(DX12SwapChain&&) = delete;
	DX12SwapChain& operator=(DX12SwapChain&&) = delete;
	
	virtual ~DX12SwapChain() = default;

	// passes the rendered image to the display and updates index / signal stuff
	void present();

	// resizes the swap chains internal back buffers
	void resize_back_buffers(uint32_t width, uint32_t height);


	D3D12Resource get_buffer_at(UINT index)
	{
		D3D12Resource buffer;
		mSwapChain->GetBuffer(index, IID_PPV_ARGS(&buffer));
		return buffer;
	}

	D3D12Resource get_current_buffer() 
	{
		return get_buffer_at(mCurrentBackBufferIndex);
	}

	D3D12_CPU_DESCRIPTOR_HANDLE get_buffer_desc_at(UINT index)const
	{
		return mDescHeap.get_descriptor_handle_for(index);
	}

	D3D12_CPU_DESCRIPTOR_HANDLE get_current_description()const
	{
		return get_buffer_desc_at(mCurrentBackBufferIndex);
	}

	UINT64 get_current_buffer_index()const { return mCurrentBackBufferIndex; }

	constexpr LONG get_width()const noexcept  { return mWidth; }
	constexpr LONG get_height()const noexcept { return mHeight; }

protected:

	// updates the back buffer views
	void reset_description_info();

private:

	// static constexpr int NUMBER_OF_BUFFERS = 3;

	//	struct BufferView
	//	{
	//		D3D12Resource buffer;
	//		UINT64 signal_value = 0; // important: all inital values are set to 0 and nowhere else are set before running
	//	};
	//	
	//	using BufferViews = std::array<BufferView, NUMBER_OF_BUFFERS>;

	// a view for device for creating RTV's and window
	ObserverPtr<ID3D12Device2> mDevice;
	const UINT64               mBufferCount;
	// the sauce
	DXGISwapChain4             mSwapChain;

	// back buffer views and current index tracker
	//	BufferViews          mBufferViews;
	UINT64               mCurrentBackBufferIndex;

	// descriptor heap and size
	DX12DescriptorHeap mDescHeap;

	// other variables
	LONG mWidth;
	LONG mHeight;
	bool mIsVSync;
	bool mIsTearingSupported;
};
