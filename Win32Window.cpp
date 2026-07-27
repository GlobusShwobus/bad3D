#include "Win32Window.h"
#include "Utils.h"
#include <stdexcept>

Win32Window::Win32Window(
	WINDOW_CREATE_DESC create_desc,
	ObserverPtr<IWindowEventListener> listener
)
{
	// bind event listener
	execute_test_throw(
		bind_event_listener(listener)
	);

	mStyle = create_desc.window_style;

	// create window
	if (!create_window( create_desc.window_name, create_desc.window_style, create_desc.x, create_desc.y, create_desc.w, create_desc.h ))
		throw_error_code_translation(::GetLastError());

	// set fullscreen or windowed
	// this will also set other internal variables: mClientWidth, mClientHeight, mWindowRect, mIsFullscreen
	if (create_desc.set_fullscreen) {
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

	// set bool fullscreen
	mIsFullscreen = true;
}

void Win32Window::set_to_windowed()
{
	// turn back on all the decor
	::SetWindowLongPtrW(mHwnd, GWL_STYLE, mStyle);

	// set the pos of the window to old pos
	::SetWindowPos(mHwnd, HWND_NOTOPMOST, mWindowedRect.x, mWindowedRect.y, mWindowedRect.w, mWindowedRect.h, SWP_FRAMECHANGED | SWP_NOACTIVATE | SWP_SHOWWINDOW);

	// set bool windowed
	mIsFullscreen = false;
}

std::unique_ptr<DX12SwapChain> Win32Window::create_swap_chain(
	ObserverPtr<IDXGIFactory4> factory,
	ObserverPtr<ID3D12Device2> device,
	ObserverPtr<ID3D12CommandQueue> command_queue,
	UINT64 number_of_buffers)
{
	return std::make_unique<DX12SwapChain>(factory, device, command_queue, ObserverPtr<std::remove_pointer_t<HWND>>(mHwnd), number_of_buffers);
}

LONG Win32Window::get_width() const noexcept
{
	RECT r;
	GetClientRect(mHwnd, &r);
	return r.right - r.left;
}
LONG Win32Window::get_height() const noexcept
{
	RECT r;
	GetClientRect(mHwnd, &r);
	return static_cast<uint32_t>(r.bottom - r.top);
}