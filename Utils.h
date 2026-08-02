#pragma once

#include "WIN32_CORE.h"

void throw_error_code_translation(DWORD error_code);
void execute_and_test_hresult(HRESULT hr);
void execute_and_test_BOOL(BOOL b);

constexpr LONG rect_width(const RECT& rect) noexcept { return rect.right - rect.left; }
constexpr LONG rect_height(const RECT& rect) noexcept { return rect.bottom - rect.top; }