#pragma once

#include "WIN32_CORE.h"
#include "GPU_CORE.h"
#include "IWin32Window.h"
#include "DX12DescriptorHeap.h"
#include "DX12CommandQueue.h"
#include "ObserverPtr.h"
#include "TypeRect.h"
#include <memory>


class DX12RenderWindow final : public IWin32Window
{
	struct BufferData
	{
		UINT   mCount;
		UINT   mCurrentIndex;
		UINT mWidth;
		UINT mHeight;
	};

public:

	DX12RenderWindow(
		WINDOW_EX_DESC desc,
		ObserverPtr<IDXGIFactory4> factory,
		ObserverPtr<ID3D12Device2> device,
		ObserverPtr<DX12CommandQueue> command_queue,
		UINT number_of_buffers);

	~DX12RenderWindow()override;

	void present();

	void on_resize();
	void on_fullscreen_transition();
	void reconfigure(WINDOW_EX_DESC desc);

	ObserverPtr<ID3D12Resource> get_buffer() const;
	D3D12_CPU_DESCRIPTOR_HANDLE get_buffer_desc()const;
	UINT                        get_buffer_index()const;

	constexpr HRESULT bind_listener(ObserverPtr<IWindowEventListener> listener) noexcept
	{
		return bind_event_listener(listener);
	}

protected:

	ObserverPtr<ID3D12Resource>  get_buffer_at(UINT index) const;

	void reset_description_info() const;

	void set_to_fullscreen();

	void set_to_windowed();

	WindowClassRegister get_class_register_info() const noexcept override;

private:

	// cache the device and command queue views required for handling resizing
	ObserverPtr<ID3D12Device2>    mDevice;
	ObserverPtr<DX12CommandQueue> mCommandQueue;

	// swapchain
	DXGISwapChain4 mSwapChain;

	// desc heap
	std::unique_ptr<DX12DescriptorHeap> mDescHeap;

	// buffer info
	BufferData mBufferData;
	
	// cached data to restore size and style between fullscreen / windowed transitions
	UIRect mWindowedRect;
	UINT   mWindowStyle;

	// settings
	bool mFullscreen;
	bool mVSync;
	bool mTearingSupported;
};