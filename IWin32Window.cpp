#include "IWin32Window.h"

bool IWin32Window::create_window(PCWSTR window_name, DWORD window_style, UINT x, UINT y, UINT w, UINT h) noexcept
{
    WindowClassRegister register_info = get_class_register_info();

    WNDCLASSEX window_desc = {};
    window_desc.cbSize        = sizeof(window_desc);
    window_desc.lpfnWndProc   = IWin32Window::wnd_proc;
    window_desc.lpszClassName = register_info.class_name;
    window_desc.style         = register_info.class_style;
    window_desc.hInstance     = register_info.module_;
    window_desc.cbClsExtra    = register_info.cbClsExtra;
    window_desc.cbWndExtra    = register_info.cbWndExtra;
    window_desc.hIcon         = register_info.hIcon;
    window_desc.hCursor       = register_info.hCursor;
    window_desc.hbrBackground = register_info.hbrBackground;
    window_desc.lpszMenuName  = register_info.lpszMenuName;
    window_desc.hIconSm       = register_info.hIconSm;

    if (RegisterClassExW(&window_desc) == 0)
    {
        if (::GetLastError() != ERROR_CLASS_ALREADY_EXISTS)
            return false;
    }

    HWND hwnd = CreateWindowExW(
        NULL,
        register_info.class_name,
        window_name,
        window_style,
        x,y,w,h,
        NULL,
        NULL,
        register_info.module_,
        this
    );

    return hwnd != nullptr;
}

bool IWin32Window::destroy() noexcept
{
    // already destroyed
    if (!mHwnd)
        return true;

    // destruction for some reason my fail (idk why but it's possible) therefor set my handle to nullptr only if it succeeds
    BOOL result = ::DestroyWindow(mHwnd);

    if (result != 0)
        mHwnd = nullptr;
    mListener = nullptr;
    return result != 0;
}