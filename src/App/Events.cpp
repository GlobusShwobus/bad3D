#include "App/Events.h"

#include "App/badWin32.h"

bool Events::pump_events() noexcept
{
    double delta = mClock.delta();
    reset_logical_events(delta);

    MSG msg = {};
    while (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE))
    {
        if (msg.message == WM_QUIT)
        {
            mState.system().exit();
            break;
        }
        DispatchMessage(&msg);
    }

    return !mState.system().is_system_quit();
}