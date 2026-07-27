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
		ObserverPtr<HWND__> client_window
	);

	DX12SwapChain(const DX12SwapChain&) = delete;
	DX12SwapChain& operator= (const DX12SwapChain&) = delete;
	DX12SwapChain(DX12SwapChain&&) = delete;
	DX12SwapChain& operator=(DX12SwapChain&&) = delete;
	
	virtual ~DX12SwapChain() = default;

	void clear(ObserverPtr<ID3D12GraphicsCommandList2> command_list, float r, float g, float b, float a)
	{
		ObserverPtr<ID3D12Resource> back_buffer = mBufferViews[mCurrentBackBufferIndex].buffer.Get();

		D3D12_RESOURCE_BARRIER barrier = {};
		barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
		barrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
		barrier.Transition.pResource = back_buffer.get();
		barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_PRESENT;         // this knowledge is implied and for back buffers it's fine because they're only ever in one or the other state. for other resources that get more complex logic, state tracking becomes important
		barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_RENDER_TARGET;
		barrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;

		// add the command to the list
		command_list->ResourceBarrier(1, &barrier);
		// index into the correct descriptor in the descriptor heap
		FLOAT color[4] = { r,g,b,a };
		command_list->ClearRenderTargetView(mDescHeap.get_descriptor_handle_for(mCurrentBackBufferIndex), color, 0, nullptr);
	}
	// passes the rendered image to the display and updates index / signal stuff
	void present(UINT64 signal_value);

	// resizes the swap chains internal back buffers
	void resize_back_buffers(uint32_t width, uint32_t height);

	// returns a command description for the command list describing transition from RTV to present state
	D3D12_RESOURCE_BARRIER command_from_rtv_to_present() const;


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

	// a view for device for creating RTV's and window
	ObserverPtr<ID3D12Device2> mDevice = nullptr;

	// the sauce
	DXGISwapChain4             mSwapChain;

	// back buffer views and current index tracker
	BufferViews          mBufferViews;
	UINT64               mCurrentBackBufferIndex;

	// descriptor heap and size
	DX12DescriptorHeap mDescHeap;

	// other variables
	LONG mWidth;
	LONG mHeight;
	bool mIsVSync;
	bool mIsTearingSupported;
};
