#pragma once

#include "App/AppState.h"
#include "Tools/Stopwatch.h"

class Events
{
public:

    bool pump_events();

    constexpr const AppState& get_state() const noexcept { return mState; }
    constexpr AppState& get_state() noexcept { return mState; }

protected:

    void reset_logical_events(double delta);
private:
    Stopwatch mClock;
    AppState mState;
};