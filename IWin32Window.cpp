#include "IWin32Window.h"

bool IWin32Window::register_class( WINDOW_REGISTER_DESC desc ) noexcept
{
    WNDCLASSEX window_desc = {};
    window_desc.cbSize = sizeof(window_desc);
    window_desc.lpfnWndProc = IWin32Window::wnd_proc;
    window_desc.lpszClassName = desc.class_name;
    window_desc.style = desc.class_style;
    window_desc.hInstance = desc.hInstance;
    window_desc.cbClsExtra = NULL;
    window_desc.cbWndExtra = NULL;
    window_desc.hIcon = NULL;
    window_desc.hCursor = NULL;
    window_desc.hbrBackground = NULL;
    window_desc.lpszMenuName = NULL;
    window_desc.hIconSm = NULL;

    // try create. can fail i think on re-registry so might be wiser to separate register and create
    return RegisterClassExW(&window_desc) == NULL ? false : true;
}

bool IWin32Window::create_window( WINDOW_CREATE_DESC desc ) noexcept
{
    HWND hwnd = CreateWindowExW(
        NULL,
        desc.class_name,
        desc.window_name,
        desc.window_style,
        desc.x,
        desc.y,
        desc.w,
        desc.h,
        NULL,
        NULL,
        desc.hInstance,
        this
    );

    return hwnd == NULL ? false : true;
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