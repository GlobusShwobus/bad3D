#pragma once

#define WIN_LEAN_AND_MEAN
#include <Windows.h>

// interface type which should be inherited by the application type or type which sits above managing the window, swapchain, commandqueue etc...
class IWindowEventListener {
public:
    virtual ~IWindowEventListener() = default;
    virtual LRESULT on_message(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) = 0;
};