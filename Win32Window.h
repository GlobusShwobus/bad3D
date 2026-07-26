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
		WINDOW_CREATE_DESC desc,
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

	void reconfigure(WINDOW_CREATE_DESC desc)
	{
		// if currently fullscreen, drop back to windowed first — otherwise we'd be
		// resizing the fullscreen borderless rect instead of the real windowed one,
		// and mWindowedRect would end up wrong for the next set_to_windowed() call.
		if (mIsFullscreen)
			set_to_windowed();

		// update the cached windowed geometry so a later set_to_windowed()/alt-tab-back
		// restores to the NEW size/position, not the previous demo's.
		mWindowedRect.x = desc.x;
		mWindowedRect.y = desc.y;
		mWindowedRect.w = desc.w;
		mWindowedRect.h = desc.h;

		// retitle
		::SetWindowTextW(mHwnd, desc.window_name);

		// reposition + resize. this fires WM_SIZE synchronously (same as any other
		// resize), which your existing on_message handler already deals with safely
		// via mCommand_queue->flush() + resize_back_buffers.
		::SetWindowPos(
			mHwnd, nullptr,
			desc.x, desc.y, desc.w, desc.h,
			SWP_NOZORDER | SWP_NOACTIVATE
		);

		// re-enter fullscreen if the new demo wants to start there
		if (desc.set_fullscreen)
			set_to_fullscreen();
	}

protected:

	WindowClassRegister get_class_register_info() const noexcept override
	{
		WindowClassRegister info = {};

		info.class_name = L"RenderWindow";
		info.module_ = g_hModule;
		info.class_style = CS_HREDRAW | CS_VREDRAW;
		info.hIcon = nullptr;
		info.hIconSm = nullptr;
		info.hCursor = nullptr;
		info.hbrBackground = nullptr;
		info.lpszMenuName = nullptr;
		info.cbClsExtra = NULL;
		info.cbWndExtra = NULL;

		return info;
	}

private:

	LRect mWindowedRect;
	UINT  mStyle;
	bool  mIsFullscreen;
};