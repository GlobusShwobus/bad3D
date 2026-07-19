#pragma once

#include "WIN32_CORE.h"

// listener interface
class IWindowEventListener
{
public:
    virtual ~IWindowEventListener() = default;
    virtual LRESULT on_message(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) = 0;
};