#include "IWin32Window.h"

bool IWin32Window::create_window( const std::wstring& title, UINT x, UINT y, UINT window_width, UINT window_height, DWORD window_style ) noexcept
{
    WNDCLASSEX register_info = get_class_register_info();

    if (RegisterClassExW(&register_info) == 0)
    {
        if (::GetLastError() != ERROR_CLASS_ALREADY_EXISTS)
            return false;
    }

    HWND hwnd = CreateWindowExW(
        NULL,
        register_info.lpszClassName,
        title.c_str(),
        window_style,
        x,y,
        window_width,
        window_height,
        NULL,
        NULL,
        register_info.hInstance,
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