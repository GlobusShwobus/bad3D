#pragma once
#include "IWin32Window.h"
#include "TypeRect.h"
#include "ObserverPtr.h"
#include <string>

#include "DX12SwapChain.h"
#include <memory>
// a slightly more useful in terms of functionality window. also does RAII

class Win32Window final : public IWin32Window
{
public:

	Win32Window(
		WINDOW_REGISTER_DESC register_desc,
		WINDOW_CREATE_DESC create_desc,
		ObserverPtr<IWindowEventListener> listener
	);

	Win32Window(const Win32Window&) = delete;
	Win32Window& operator=(const Win32Window&) = delete;
	Win32Window(Win32Window&&) noexcept = delete;
	Win32Window& operator=(Win32Window&&) noexcept = delete;

	virtual ~Win32Window() noexcept;

	constexpr bool is_fullscreen() const noexcept { return mIsFullscreen; }
	LONG get_width() const noexcept;
	LONG get_height() const noexcept;

	std::unique_ptr<DX12SwapChain> create_swap_chain(
		ObserverPtr<IDXGIFactory4> factory,
		ObserverPtr<ID3D12Device2> device,
		ObserverPtr<ID3D12CommandQueue> command_queue);

	void set_to_fullscreen();

	void set_to_windowed();

private:

	LRect mWindowedRect;
	UINT  mStyle;
	bool  mIsFullscreen;
};