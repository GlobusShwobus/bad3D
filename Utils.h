#pragma once

#include "badWin32.h"

void throw_error_code_translation(DWORD error_code);
void execute_and_test_hresult(HRESULT hr);
void execute_and_test_BOOL(BOOL b);

template <typename TypeRect>
constexpr auto rect_width(const TypeRect& rect) noexcept { return rect.right - rect.left; }
template <typename TypeRect>
constexpr auto rect_height(const TypeRect& rect) noexcept { return rect.bottom - rect.top; }