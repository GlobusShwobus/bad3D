#pragma once

#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#include <string>
#include <stdexcept>

void throw_error_code_translation(DWORD error_code)
{
	// TODO:: add maybe a message box
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

void adjust_desired_client_and_window_size(int in_width, int in_height, int& out_x, int& out_y, int& out_width, int& out_height)
{
	RECT client_area_rect = { 0, 0, in_width, in_height };
	// adjust the client area to full area of the window including edges
	::AdjustWindowRect(&client_area_rect, WS_OVERLAPPEDWINDOW, FALSE);
	out_width = int(client_area_rect.right - client_area_rect.left);
	out_height = int(client_area_rect.bottom - client_area_rect.top);

	// using the phyisical display size offset to middle of the screen
	int sys_width = ::GetSystemMetrics(SM_CXSCREEN);
	int sys_height = ::GetSystemMetrics(SM_CYSCREEN);
	out_x = std::max<int>(0, (sys_width - out_width) / 2);
	out_y = std::max<int>(0, (sys_height - out_height) / 2);
}