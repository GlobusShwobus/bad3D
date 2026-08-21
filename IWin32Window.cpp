#include "IWin32Window.h"

HWND IWin32Window::create_window(LPCWSTR name, int x, int y, int window_width, int window_height, DWORD window_style ) noexcept
{
    WNDCLASSEX register_info = get_class_register_info();
    register_info.lpfnWndProc = IWin32Window::wnd_proc;

    if (RegisterClassExW(&register_info) == 0)
    {
        if (::GetLastError() != ERROR_CLASS_ALREADY_EXISTS)
            return nullptr;
    }

    HWND hwnd = CreateWindowExW(
        NULL,
        register_info.lpszClassName,
        name,
        window_style,
        x,
        y,
        window_width,
        window_height,
        NULL,
        NULL,
        register_info.hInstance,
        this
    );

    return hwnd;
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

    return result != 0;
}