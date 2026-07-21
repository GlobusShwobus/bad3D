#include "Win32Window.h"
#include "Utils.h"

Win32Window::Win32Window(
	const std::wstring& window_name,
	uint32_t client_width,
	uint32_t client_height, 
	bool fullscreen_mode,
	ObserverPtr<IWindowEventListener> listener, 
	DWORD window_style
)
{
	const std::wstring class_name = L"Win32Window class";
	const DWORD class_style = CS_HREDRAW | CS_VREDRAW;
	HINSTANCE hInstance = ::GetModuleHandleW(nullptr);
	mStyle = window_style;
	// try register
	if (!register_class(class_name.c_str(), class_style, hInstance))
		throw_error_code_translation(::GetLastError());

	// always start off from windowed size, whatever was input
	LRect size = get_adjusted_window_rect(client_width, client_height, window_style);
	center_rect_in_display(size);

	// create window
	if (!create_window(class_name.c_str(), window_name.c_str(), window_style, size.x, size.y, size.w, size.h, hInstance, listener))
		throw_error_code_translation(::GetLastError());

	// set fullscreen or windowed
	// this will also set other internal variables: mClientWidth, mClientHeight, mWindowRect, mIsFullscreen
	if (fullscreen_mode) {
		set_to_fullscreen();
	}
	else
	{
		mWindowedRect = query_window_rect(mHwnd); // the first time this runs and in case set_to_windowed is true mWindowRect would never be set
		set_to_windowed();
	}
}

Win32Window::~Win32Window() noexcept
{
	// rarely, though it never realistically should, destroy can return false meaning failure.
	// todo: do not let the destructor fail. somehow log the error and move on but maybe just pray it never breaks, which it shouldn't!
	destroy();
}

void Win32Window::set_to_fullscreen()
{
	// cache windowed size
	mWindowedRect = query_window_rect(mHwnd);

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
	::SetWindowPos(mHwnd, HWND_TOP, x, y, w, h, SWP_FRAMECHANGED | SWP_NOACTIVATE | SWP_SHOWWINDOW);

	// also cache the real current w / h
	mClientWidth = w;
	mClientHeight = h;

	// set bool fullscreen
	mIsFullscreen = true;
}

void Win32Window::set_to_windowed()
{
	// turn back on all the decor
	::SetWindowLongPtrW(mHwnd, GWL_STYLE, mStyle);

	// set the pos of the window to old pos
	::SetWindowPos(mHwnd, HWND_NOTOPMOST, mWindowedRect.x, mWindowedRect.y, mWindowedRect.w, mWindowedRect.h, SWP_FRAMECHANGED | SWP_NOACTIVATE | SWP_SHOWWINDOW);

	// also cache the width / height
	query_client_size(mHwnd, mClientWidth, mClientHeight);

	// set bool windowed
	mIsFullscreen = false;
}

void Win32Window::process_window_message(UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	// This function is supplementary, processing messages relating to this class.
	// It is not meant to take over or drain messages which is the job of the event listener.
	// It's really just a convenience and encapsulation function, enabling making other related methods private.

	switch (uMsg)
	{
	case WM_SIZE:
		query_client_size(mHwnd, mClientWidth, mClientHeight);
		break;

	case WM_SYSKEYDOWN:
	case WM_KEYDOWN:

		if (wParam == VK_F11)
		{
			if (mIsFullscreen)
				set_to_windowed();
			else
				set_to_fullscreen();
		}

		break;

	default:
		break;
	}
}