#pragma once

#include "badWin32.h"

class IWin32Window
{
protected:

    IWin32Window() = default;
    
    // class register info. overwrites callback function.
    virtual WNDCLASSEX get_class_register_info() const noexcept = 0;
    virtual LRESULT on_message(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) = 0;

    // if nullptr call ::GetLastError
    HWND create_window(LPCWSTR name, int x, int y, int window_width, int window_height, DWORD window_style) noexcept;

    // destroys window and all context. returns true on success, false on failure. call GetLastError on failure.
    bool destroy() noexcept;

    // win32 bullshit. i hate it so bad.
    static LRESULT CALLBACK wnd_proc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
    {
        IWin32Window* self = nullptr;
        if (uMsg == WM_NCCREATE) {
            CREATESTRUCT* pCreate = (CREATESTRUCT*)lParam;
            self = (IWin32Window*)pCreate->lpCreateParams;
            SetWindowLongPtrW(hwnd, GWLP_USERDATA, (LONG_PTR)self);
            self->mHwnd = hwnd;
        }
        else {
            self = (IWin32Window*)GetWindowLongPtrW(hwnd, GWLP_USERDATA);
        }

        if (self)
            return self->on_message(hwnd, uMsg, wParam, lParam);

        return DefWindowProcW(hwnd, uMsg, wParam, lParam);
    }

    HWND mHwnd = nullptr;

public:

    virtual ~IWin32Window() = default;
};