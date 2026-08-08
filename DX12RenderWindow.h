#pragma once

#include "WIN32_CORE.h"
#include "GPU_CORE.h"
#include "IWin32Window.h"
#include "DX12DescriptorHeap.h"
#include "DX12CommandQueue.h"
#include "ObserverPtr.h"

#include <memory>
#include <string>

class DX12RenderWindow final : public IWin32Window
{
	static constexpr UINT back_buffer_count = 3;
public:

	DX12RenderWindow(
		const std::wstring& title, UINT x, UINT y, UINT client_width, UINT client_height, DWORD window_style,
		ObserverPtr<IDXGIFactory4> factory,
		ObserverPtr<ID3D12Device4> device,
		ObserverPtr<ID3D12CommandQueue> command_queue,
		ObserverPtr<IWindowEventListener> listener
	);

	DX12RenderWindow(const DX12RenderWindow&) = delete;
	DX12RenderWindow& operator=(const DX12RenderWindow&) = delete;
	DX12RenderWindow(DX12RenderWindow&&) = delete;
	DX12RenderWindow& operator=(DX12RenderWindow&&) = delete;

	~DX12RenderWindow()override;

	void present_to_display();
	void resize(const UINT cwidth, const UINT cheight);
	void toggle_fullscreen(bool fullscreen);

	ObserverPtr<ID3D12Resource> get_buffer() const;
	D3D12_CPU_DESCRIPTOR_HANDLE get_buffer_desc()const;
	UINT                        get_buffer_index()const;
	void                        get_client_size(UINT& width, UINT& height)const;

protected:

	ObserverPtr<ID3D12Resource>  get_buffer_at(UINT index) const;

	void reset_description_info() const;

	WNDCLASSEX get_class_register_info() const noexcept override;

private:

	// cache the device and command queue views required for handling resizing
	ObserverPtr<ID3D12Device4>    mDevice;

	// swapchain
	DXGISwapChain4 mSwapChain;

	// desc heap
	DX12DescriptorHeap mDescHeap;

	// buffer info
	UINT mCurrentBBIndex;
	UINT mBBWidth;
	UINT mBBHeight;
	
	// cached data to restore size and style between fullscreen / windowed transitions
	RECT   mWindowedRect;
	UINT   mWindowStyle;

	// settings
	bool mIsFullscreen;
	bool mIsVSync;
	bool mIsTearingSupported;
};