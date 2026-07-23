#pragma once

#include "GPU_CORE.h"
#include "ObserverPtr.h"
#include <array>

class DX12SwapChain final
{
public:
	DX12SwapChain(
		ObserverPtr<IDXGIFactory4> factory,
		ObserverPtr<ID3D12Device2> device,
		ObserverPtr<ID3D12CommandQueue> command_queue,
		ObserverPtr<HWND__> client_window
	);

	DX12SwapChain(const DX12SwapChain&) = delete;
	DX12SwapChain& operator= (const DX12SwapChain&) = delete;
	DX12SwapChain(DX12SwapChain&&) = delete;
	DX12SwapChain& operator=(DX12SwapChain&&) = delete;
	
	virtual ~DX12SwapChain() = default;

	// passes the rendered image to the display and updates index / signal stuff
	void present(UINT64 signal_value);

	// resizes the swap chains internal back buffers
	void resize_back_buffers(uint32_t width, uint32_t height);

	// returns a command description for the command list describing transition from present state to RTV state
	D3D12_RESOURCE_BARRIER command_from_present_to_rtv() const;

	// returns a command description for the command list describing transition from RTV to present state
	D3D12_RESOURCE_BARRIER command_from_rtv_to_present() const;

	// returns a command for the command list containing the back buffer descriptor
	D3D12_CPU_DESCRIPTOR_HANDLE command_backbuffer_handle() const;

	constexpr UINT64 get_current_buffer_signal_value()const noexcept { return mBufferViews[mCurrentBackBufferIndex].signal_value; }
	constexpr LONG get_width()const noexcept { return mWidth; }
	constexpr LONG get_height()const noexcept { return mHeight; }


protected:

	// updates the back buffer views
	void update_back_buffers_views();

private:

	static constexpr int NUMBER_OF_BUFFERS = 3;

	struct BufferView
	{
		D3D12Resource buffer;
		UINT64 signal_value = 0; // important: all inital values are set to 0 and nowhere else are set before running
	};

	using BufferViews = std::array<BufferView, NUMBER_OF_BUFFERS>;

	// a view for device for creating RTV's
	ObserverPtr<ID3D12Device2> mDevice = nullptr;

	// the sauce
	DXGISwapChain4             mSwapChain;

	// back buffer views and current index tracker
	BufferViews          mBufferViews;
	UINT64               mCurrentBackBufferIndex;

	// descriptor heap and size
	D3D12DescriptorHeap   mDescriptorHeap;
	UINT                  mDescriptorSize;

	// other variables
	LONG mWidth;
	LONG mHeight;
	bool mIsVSync;
	bool mIsTearingSupported;
};
