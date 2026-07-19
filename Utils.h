#pragma once

#include "WIN32_CORE.h"
#include "TypePoint.h"
#include "TypeRect.h"

void throw_error_code_translation(DWORD error_code);

void execute_test_throw(HRESULT hr);

LRect get_adjusted_window_rect(int clinet_width, int client_height, DWORD window_style);

void center_rect_in_display(LRect& rect);

LRect get_display_rect_from_cursor();

LRect query_window_rect(HWND window);

LRect query_client_rect(HWND window);

void query_client_size(HWND window, LONG& out_width, LONG& out_height);