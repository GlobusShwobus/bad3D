#pragma once
#include "IWin32Window.h"
#include "TypeRect.h"
#include "Utils.h"
#include <string>

class Win32Window : public IWin32Window
{
public:

	Win32Window(const std::wstring& window_name, int w, int h, bool fullscreen_mode)
	{
		const std::wstring class_name = L"Win32Window class";
		const DWORD class_style       = CS_HREDRAW | CS_VREDRAW;
		const DWORD window_style      = WS_OVERLAPPEDWINDOW;
		HINSTANCE hInstance           = ::GetModuleHandleW(nullptr);

		if (!register_class(class_name.c_str(), class_style, hInstance))
			throw_error_code_translation(::GetLastError());

		if(!create_window(class_name.c_str(), window_name.c_str(), ))
			throw_error_code_translation(::GetLastError());
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

	LRect client_size_to_full_size(int in_width, int in_height) const
	{
		LRect result;
		RECT client_area_rect = RECT{ 0, 0, in_width, in_height };
		// adjust the client area to full area of the window including edges
		::AdjustWindowRect(&client_area_rect, mStyle, FALSE);
		result.w = client_area_rect.right - client_area_rect.left;
		result.h = client_area_rect.bottom - client_area_rect.top;

		// using the phyisical display size offset to middle of the screen
		int sys_width = ::GetSystemMetrics(SM_CXSCREEN);
		int sys_height = ::GetSystemMetrics(SM_CYSCREEN);
		result.x = std::max<LONG>(0, (sys_width - result.w) / 2);
		result .y = std::max<LONG>(0, (sys_height - result.h) / 2);

		return result;
	}

private:
	LRect mWindowedRect;
	UINT32 mWidth;
	UINT32 mHeight;
	UINT mStyle;
	bool mIsFullscreen;
};