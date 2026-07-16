#pragma once

#define WIN32_LEAN_AND_MEAN
#include <Windows.h>

void throw_error_code_translation(DWORD error_code);

void execute_test_throw(HRESULT hr);

void adjust_desired_client_and_window_size(int in_width, int in_height, int& out_x, int& out_y, int& out_width, int& out_height);