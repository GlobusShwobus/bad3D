#pragma once
#include "DX12SwapChain.h"
#include "IWin32Window.h"
#include "TypeRect.h"
#include "Utils.h"
#include <string>

// a slightly more useful in terms of functionality window. also does RAII

class Win32Window : public IWin32Window
{
public:

	Win32Window(const std::wstring& window_name, uint32_t client_width, uint32_t client_height, bool fullscreen_mode, IWindowEventListener* listener)
	{
		const std::wstring class_name = L"Win32Window class";
		const DWORD class_style       = CS_HREDRAW | CS_VREDRAW;
		HINSTANCE hInstance           = ::GetModuleHandleW(nullptr);

		// try register
		if (!register_class(class_name.c_str(), class_style, hInstance))
			throw_error_code_translation(::GetLastError());

		// determine size
		LRect size;
		DWORD window_style;
		if (fullscreen_mode)
		{
			size = get_window_size_from_display_mousepos();
			window_style = 0;
			mWidth = size.w;
			mHeight = size.h;
		}
		else
		{
			size = get_window_size_from_client_size(client_width, client_height, WS_OVERLAPPEDWINDOW);
			window_style = WS_OVERLAPPEDWINDOW;
			mWidth = client_width;
			mHeight = client_height;
		}

		// create window
		if(!create_window(class_name.c_str(), window_name.c_str(), window_style, size.x,size.y,size.w,size.h, hInstance, listener))
			throw_error_code_translation(::GetLastError());

		// set local variables
		mWindowedRect = size;
		mStyle = window_style;
		mIsFullscreen = fullscreen_mode;
	}

	Win32Window(const Win32Window&) = delete;
	Win32Window& operator=(const Win32Window&) = delete;
	Win32Window(Win32Window&& rhs)noexcept = delete;
	Win32Window& operator=(Win32Window&& rhs) noexcept = delete;

	~Win32Window()
	{
		if (mHwnd)
			destroy();
	}

	void show_window()
	{
		::ShowWindow(mHwnd, mIsFullscreen ? SW_MAXIMIZE : SW_NORMAL);
	}

	void set_to_fullscreen()
	{
		if (!mIsFullscreen)
		{
			// cache windowed size
			save_current_windowed_size();

			// remove all decoration
			const UINT windowStyle = 0;

			// change the window style attribute of the window with the given style above
			::SetWindowLongPtrW(mHwnd, GWL_STYLE, windowStyle);

			// query the name of the nearest display monitor and set fullscreen to the dominant one (if multi monitor)
			HMONITOR hMonitor = ::MonitorFromWindow(mHwnd, MONITOR_DEFAULTTONEAREST);
			MONITORINFOEX monitorinfo = {};
			monitorinfo.cbSize = sizeof(MONITORINFOEX);
			::GetMonitorInfo(hMonitor, &monitorinfo);

			// set the position of the window and make the window top-most
			int x, y, w, h;
			x = monitorinfo.rcMonitor.left;
			y = monitorinfo.rcMonitor.top;
			w = monitorinfo.rcMonitor.right - monitorinfo.rcMonitor.left;
			h = monitorinfo.rcMonitor.bottom - monitorinfo.rcMonitor.top;
			::SetWindowPos(mHwnd, HWND_TOP, x, y, w, h, SWP_FRAMECHANGED | SWP_NOACTIVATE);

			// also cache the real current w / h
			mWidth = w;
			mHeight = h;

			// set bool fullscreen
			mIsFullscreen = true;
		}
	}

	void set_to_windowed()
	{
		if (mIsFullscreen)
		{
			// turn back on all the decor
			::SetWindowLongPtrW(mHwnd, GWL_STYLE, mStyle);

			// set the pos of the window to old pos
			::SetWindowPos(mHwnd, HWND_NOTOPMOST, mWindowedRect.x, mWindowedRect.y, mWindowedRect.w, mWindowedRect.h, SWP_FRAMECHANGED | SWP_NOACTIVATE);

			// also cache the width / height
			mWidth = mWindowedRect.w;
			mHeight = mWindowedRect.h;

			// set bool windowed
			mIsFullscreen = false;
		}
	}

	bool is_fullscreen()const { return mIsFullscreen; }
	UINT32 get_width()const { return mWidth; }
	UINT32 get_height()const { return mHeight; }
	HWND get_HWND()const { return mHwnd; }

private:

	void save_current_windowed_size()
	{
		// query the current window pos and size then cache it to be able to restore the windowed pos and size
		RECT current_rect;
		::GetWindowRect(mHwnd, &current_rect);

		mWindowedRect.x = current_rect.left;
		mWindowedRect.y = current_rect.top;
		mWindowedRect.w = current_rect.right - current_rect.left;
		mWindowedRect.h = current_rect.bottom - current_rect.top;
	}

	LRect get_window_size_from_client_size(int in_width, int in_height, DWORD style) const
	{
		LRect result;
		RECT client_area_rect = RECT{ 0, 0, in_width, in_height };
		// adjust the client area to full area of the window including edges
		::AdjustWindowRect(&client_area_rect, style, FALSE);
		result.w = client_area_rect.right - client_area_rect.left;
		result.h = client_area_rect.bottom - client_area_rect.top;

		// using the phyisical display size offset to middle of the screen
		int sys_width = ::GetSystemMetrics(SM_CXSCREEN);
		int sys_height = ::GetSystemMetrics(SM_CYSCREEN);
		result.x = std::max<LONG>(0, (sys_width - result.w) / 2);
		result .y = std::max<LONG>(0, (sys_height - result.h) / 2);

		return result;
	}

	LRect get_window_size_from_display_mousepos()
	{
		//query where the mouse is to determine the monitor: for example if the exe is in monitor2 therefor mouse is also in monitor2 to click it..
		POINT cursor_pos;
		::GetCursorPos(&cursor_pos);
		// query the display monitor from the mouse cursor
		HMONITOR hMonitor = ::MonitorFromPoint(cursor_pos, MONITOR_DEFAULTTONEAREST);
		MONITORINFOEX monitorinfo = {};
		monitorinfo.cbSize = sizeof(MONITORINFOEX);
		::GetMonitorInfoW(hMonitor, &monitorinfo);

		return LRect{
			monitorinfo.rcMonitor.left,
			monitorinfo.rcMonitor.top,
			monitorinfo.rcMonitor.right - monitorinfo.rcMonitor.left,
			monitorinfo.rcMonitor.bottom - monitorinfo.rcMonitor.top
		};
	}

private:
	LRect mWindowedRect;
	UINT32 mWidth;
	UINT32 mHeight;
	UINT mStyle;
	bool mIsFullscreen;
};