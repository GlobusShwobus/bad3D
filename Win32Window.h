#pragma once
#include "IWin32Window.h"
#include "TypeRect.h"
#include "Utils.h"
#include <string>

class Win32Window : public IWin32Window
{
public:

	Win32Window()
	{
		const std::wstring class_name = L"Win32Window class";
	}


	void set_to_fullscreen()
	{
		if (!mIsFullscreen)
		{
			// store the window rect so they can be used to get back to windowed mode (also sets x,y that's who this func)
			RECT tempRect;
			::GetWindowRect(mHwnd, &tempRect);

			// remove all decoration
			const UINT windowStyle = 0;

			// change the window style attribute of the window with the given style above
			::SetWindowLongPtrW(mHwnd, GWL_STYLE, windowStyle);

			// query the name of the nearest display device for the window and set full screen to the dominant screen. not both or any other weird way
			// get info
			HMONITOR hMonitor = ::MonitorFromWindow(mHwnd, MONITOR_DEFAULTTONEAREST);
			MONITORINFOEX monitorinfo = {};
			monitorinfo.cbSize = sizeof(MONITORINFOEX);
			::GetMonitorInfoW(hMonitor, &monitorinfo);

			// set the position of the window (potentially calmping to one monitor if there are more than 1 monitors)
			int x, y, w, h;
			x = monitorinfo.rcMonitor.left;
			y = monitorinfo.rcMonitor.top;
			w = monitorinfo.rcMonitor.right - monitorinfo.rcMonitor.left;
			h = monitorinfo.rcMonitor.bottom - monitorinfo.rcMonitor.top;
			::SetWindowPos(mWindow.get(), HWND_TOP, x, y, w, h, SWP_FRAMECHANGED | SWP_NOACTIVATE);

			::ShowWindow(mWindow.get(), SW_MAXIMIZE);
		}
	}

	bool is_fullscreen()const { return mIsFullscreen; }
	UINT32 get_width()const { return mWidth; }
	UINT32 get_height()const { return mHeight; }
private:
	LRect mWindowedRect;
	UINT32 mWidth;
	UINT32 mHeight;
	UINT mStyle;
	bool mIsFullscreen;
};