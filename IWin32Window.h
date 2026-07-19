#pragma once

#include "WIN32_CORE.h"
#include "IWindowEventListener.h"
#include "ObserverPtr.h"

// win32 window interface. does not do automatic clean up
// NOTE: when create_window(...) is called the internal windows API will fire the first wnd_poc immediately meaning at least
//       one wnd_proc runs before the scope of create_window(...) finishes

class IWin32Window
{
public:
    virtual ~IWin32Window() = default;
protected:
    
    // listener is DX12Applications listener. this member is glue to make win32 static window procedure work
    ObserverPtr<IWindowEventListener> mListener = nullptr;

    // window handle. no automatic delete
    HWND mHwnd = nullptr;

    // default constructor
    IWin32Window() = default;

    // register the window
    bool register_class( PCWSTR class_name, DWORD class_style, HINSTANCE hInstance ) noexcept;

    // creates window. if returns false call GetLastError
    bool create_window(
        PCWSTR class_name,
        PCWSTR window_name,
        DWORD window_style,
        int x,
        int y,
        int w,
        int h,
        HINSTANCE hInstance,
        ObserverPtr<IWindowEventListener> listener
    ) noexcept;

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

        if (self && self->mListener)
            return self->mListener->on_message(hwnd, uMsg, wParam, lParam);

        return DefWindowProcW(hwnd, uMsg, wParam, lParam);
    }
};