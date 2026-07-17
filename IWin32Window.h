#pragma once

#define WIN_LEAN_AND_MEAN
#include <Windows.h>
#include "IWindowEventListener.h"
#include "ObserverPtr.h"

// NOTE: this class does not destroy the window automatically... yet

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
    IWin32Window() :mHwnd(nullptr), mListener(nullptr) {}

    // register the window
    bool register_class(
        PCWSTR class_name,
        DWORD class_style,
        HINSTANCE hInstance
    )
    {
        WNDCLASSEX window_desc = {};
        window_desc.cbSize = sizeof(window_desc);
        window_desc.lpfnWndProc = IWin32Window::wnd_proc;
        window_desc.lpszClassName = class_name;
        window_desc.style = class_style;
        window_desc.hInstance = hInstance;
        window_desc.cbClsExtra = NULL;
        window_desc.cbWndExtra = NULL;
        window_desc.hIcon = NULL;
        window_desc.hCursor = NULL;
        window_desc.hbrBackground = NULL;
        window_desc.lpszMenuName = NULL;
        window_desc.hIconSm = NULL;

        // try create. can fail i think on re-registry so might be wiser to separate register and create
        return RegisterClassExW(&window_desc);
    }

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
        IWindowEventListener* listener
    )
    {
        // wnd_proc will reassign handle. that's why mHwnd is not assigned to here. not an error but not 100% sexy either
        if (!CreateWindowExW(
            NULL,
            class_name,
            window_name,
            window_style,
            x,
            y,
            w,
            h,
            NULL,
            NULL,
            hInstance,
            this
        ))
            return false;

        mListener.observe_this(listener);

        return true;
    }

    // destroys window and all context. returns 0 on fail. call GetLastError for more info on failure.
    BOOL destroy()const { return ::DestroyWindow(mHwnd); }

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