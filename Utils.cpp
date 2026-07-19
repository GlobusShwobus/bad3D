#include "Utils.h"
#include <string>
#include <stdexcept>

void throw_error_code_translation(DWORD error_code)
{
	// TODO:: add maybe a message box
	// TODO:: add maybe a stack trace
	LPVOID lpMsgBuf = nullptr;

	DWORD len = FormatMessageA(
		FORMAT_MESSAGE_ALLOCATE_BUFFER |
		FORMAT_MESSAGE_FROM_SYSTEM |
		FORMAT_MESSAGE_IGNORE_INSERTS,
		NULL,
		error_code,
		MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
		(LPSTR)&lpMsgBuf,
		0, NULL
	);

	if (len == 0 || lpMsgBuf == nullptr)
		throw std::runtime_error("unknown error (code " + std::to_string(error_code) + ")");

	std::string msg((LPSTR)lpMsgBuf);
	LocalFree(lpMsgBuf);
	throw std::runtime_error(msg);
}

void execute_test_throw(HRESULT hr)
{
	if (FAILED(hr))
		throw_error_code_translation(static_cast<DWORD>(hr));
}

LRect get_adjusted_window_rect(int clinet_width, int client_height, DWORD window_style)
{
	LRect result;
	RECT client_area_rect = RECT{ 0, 0, clinet_width, client_height };
	// adjust the client area to full area of the window including edges
	::AdjustWindowRect(&client_area_rect, window_style, FALSE);
	result.w = client_area_rect.right - client_area_rect.left;
	result.h = client_area_rect.bottom - client_area_rect.top;
	result.x = 0;
	result.y = 0;

	return result;
}

void center_rect_in_display(LRect& rect)
{
	// using the phyisical display size offset to middle of the screen
	const int display_w = ::GetSystemMetrics(SM_CXSCREEN);
	const int display_h = ::GetSystemMetrics(SM_CYSCREEN);
	rect.x = std::max<LONG>(0, (display_w - rect.w) / 2);
	rect.y = std::max<LONG>(0, (display_h - rect.h) / 2);
}

LRect get_display_rect_from_cursor()
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

LRect query_window_rect(HWND window)
{
	RECT window_rect;
	::GetWindowRect(window, &window_rect);

	return LRect{
		window_rect.left,
		window_rect.top,
		window_rect.right - window_rect.left,
		window_rect.bottom - window_rect.top
	};
}

LRect query_client_rect(HWND window)
{
	RECT client_rect;
	::GetClientRect(window, &client_rect);

	return LRect{
		client_rect.left,  
		client_rect.top,   
		client_rect.right - client_rect.left,
		client_rect.bottom - client_rect.top
	};
}

void query_client_size(HWND window, LONG& out_width, LONG& out_height)
{
	RECT client_rect;
	::GetClientRect(window, &client_rect);
	out_width = client_rect.right - client_rect.left;
	out_height = client_rect.bottom - client_rect.top;
}
