#pragma once

#include "WIN32_CORE.h"
#include "IWindowEventListener.h"
#include "ObserverPtr.h"

#include <string>

// Interface window.
// The child class that inherits from this class must define get_class_register_info for registering the info.
// Registering the class is mostly entirely customizable EXCEPT for the window proc. 
// Any thing that inherits from this window interface is therefor not really the true window message listener.
// To customize how OS messages are handled either bind an event listener with bind_event_listener function or
// do multiple inheritence ( class UIWindow: public IWin32Window, public IWindowEventListener ) and manually set mListener.
// The interface class DOES NOT OWN mListener.

class IWin32Window
{
public:

    virtual ~IWin32Window() = default;

protected:

    // default constructor
    IWin32Window() = default;
    
    // class appearance info for class registration. everything is configurable except window proc. internally create_window will ignore users window proc and overwrite it
    virtual WNDCLASSEX get_class_register_info() const noexcept = 0;

    // creates window. if returns false call GetLastError
    bool create_window(const std::wstring& title, UINT x, UINT y, UINT window_width, UINT window_height, DWORD window_style) noexcept;

    // destroys window and all context. returns true on success, false on failure. call GetLastError on failure.
    bool destroy() noexcept;

    // bind event listener. with the listener interface the user may customize message responses
    constexpr HRESULT bind_event_listener(ObserverPtr<IWindowEventListener> listener) noexcept
    {
        if (!listener)
            return E_POINTER;

        mListener = listener;
        return S_OK;
    }

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

    // listener is DX12Applications listener. this member is glue to make win32 static window procedure work
    ObserverPtr<IWindowEventListener> mListener = nullptr;

    // window handle. no automatic delete
    HWND mHwnd = nullptr;
};