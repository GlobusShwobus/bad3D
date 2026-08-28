#pragma once

#include "badWin32.h"
#include <string>

struct AppWinDesc
{
	std::wstring window_name;
	DWORD window_style = 0;
	HINSTANCE hInstance = nullptr;

	HICON hIcon = nullptr;
	HICON hIconSm = nullptr;
	HCURSOR hCursor = nullptr;

	int x = 0;
	int y = 0;
	int cw = 0;
	int ch = 0;
};