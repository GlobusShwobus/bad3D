#include "IWin32Window.h"

bool IWin32Window::register_class(
    PCWSTR class_name,
    DWORD class_style,
    HINSTANCE hInstance
) noexcept
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
    return RegisterClassExW(&window_desc) == NULL ? false : true;
}

bool IWin32Window::create_window(
    PCWSTR class_name,
    PCWSTR window_name,
    DWORD window_style,
    int x,
    int y,
    int w,
    int h,
    HINSTANCE hInstance,
    ObserverPtr<IWindowEventListener> listener
) noexcept
{
    mListener.observe_this(listener.get());

    HWND hwnd = CreateWindowExW(
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

    return result != 0;
}