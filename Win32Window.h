#pragma once
#include "IWin32Window.h"
#include "TypeRect.h"
#include "ObserverPtr.h"
#include <string>

// a slightly more useful in terms of functionality window. also does RAII

class Win32Window : public IWin32Window
{
public:

	Win32Window(
		const std::wstring& window_name,
		uint32_t client_width,
		uint32_t client_height,
		bool fullscreen_mode,
		ObserverPtr<IWindowEventListener> listener,
		DWORD window_style = WS_OVERLAPPEDWINDOW
	);

	Win32Window(const Win32Window&) = delete;
	Win32Window& operator=(const Win32Window&) = delete;
	Win32Window(Win32Window&&) noexcept = delete;
	Win32Window& operator=(Win32Window&&) noexcept = delete;

	virtual ~Win32Window() noexcept;

	void process_window_message(UINT uMsg, WPARAM wParam, LPARAM lParam);

	constexpr bool is_fullscreen() const noexcept { return mIsFullscreen; }
	constexpr LONG get_width() const noexcept     { return mClientWidth; }
	constexpr LONG get_height() const noexcept    { return mClientHeight; }

	constexpr HWND get_HWND() const noexcept      { return mHwnd; } // ideally get rid of this

private:

	void set_to_fullscreen();

	void set_to_windowed();

private:

	LRect mWindowedRect;
	LONG  mClientWidth;
	LONG  mClientHeight;
	UINT  mStyle;
	bool  mIsFullscreen;
};