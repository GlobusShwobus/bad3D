#pragma once

#include "Tools/Stopwatch.h"

enum class MouseButtonType : unsigned int
{
    Left,
    Right,
    Middle,
    Count
};

class ClockState
{
public:
    constexpr double delta() const noexcept { return mDeltaAge; }
    constexpr double age() const noexcept { return mTotalAge; }

    constexpr void update(double delta) noexcept { mTotalAge += delta; mDeltaAge = delta; }

private:
    double mDeltaAge = 0.0;
    double mTotalAge = 0.0;
};

class WindowState
{
public:
    constexpr bool is_resized() const noexcept { return mEvent; }
    constexpr int  width() const noexcept { return mWidth; }
    constexpr int  height() const noexcept { return mHeight; }

    constexpr void resize(int w, int h) noexcept { mEvent = true; mWidth = w; mHeight = h; }
    constexpr void reset() noexcept { mEvent = false; }

private:
    int mWidth = 0;
    int mHeight = 0;
    bool mEvent = false;
};

class SystemState
{
public:
    // getters, viewed globally
    constexpr bool is_system_quit() const noexcept { return mSysQuit; }

    // setters, only seen in app
    constexpr bool exit() noexcept { return mSysQuit = true; }

private:
    bool mSysQuit = false;
};

class KeyboardState
{
public:
    // getters, viewed globally
    const bool* keys() const noexcept { return mKeys; }
    const bool is_key_pressed(unsigned char key) const { return mKeys[key]; }

    // setters, only seen in app
    constexpr void set_up(unsigned long long param) noexcept { mKeys[param] = false; }
    constexpr void set_down(unsigned long long param) noexcept { mKeys[param] = true; }

private:
    bool mKeys[256] = { false };
};

class MousePos
{
public:
    constexpr int x() const noexcept { return mPosX; }
    constexpr int y() const noexcept { return mPosY; }

    constexpr void set(int x, int y) noexcept { mPosX = x; mPosY = y; }
private:
    int mPosX = 0;
    int mPosY = 0;
};

class MouseHover
{
public:
    constexpr double duration() const noexcept { return mHoverDuration; }

    constexpr void update(double delta) noexcept { mHoverDuration += delta; }
    constexpr void reset() noexcept { mHoverDuration = 0.0f; }

private:
    double mHoverDuration = 0.0;
};

class MouseWheel
{
public:
    constexpr int units() const noexcept { return mWheelDelta; }
    constexpr float normalized() const noexcept { return mWheelDeltaNormalized; }

    constexpr void set(int delta, int unit_delta_value) noexcept { mWheelDelta = delta; mWheelDeltaNormalized = delta / static_cast<float>(unit_delta_value); }
    constexpr void reset() noexcept { mWheelDelta = 0; mWheelDeltaNormalized = 0; }
private:

    int mWheelDelta = 0;
    float mWheelDeltaNormalized = 0.0f;
};

class MouseButton
{
public:

    constexpr bool is_down(MouseButtonType button) const noexcept { return mButtons[(unsigned long long)(button)] == true; }

    constexpr void set_up(MouseButtonType button) noexcept { mButtons[(unsigned long long)(button)] = false; }
    constexpr void set_down(MouseButtonType button) noexcept { mButtons[(unsigned long long)(button)] = true; }

private:
    bool mButtons[(unsigned long long)(MouseButtonType::Count)] = { false };
};

class MouseState
{
public:
    constexpr const MousePos& position() const noexcept { return mPos; }
    constexpr const MouseHover& hover() const noexcept { return mHover; }
    constexpr const MouseWheel& wheel() const noexcept { return mWheel; }
    constexpr const MouseButton& button() const noexcept { return mButton; }

    constexpr MousePos& position() noexcept { return mPos; }
    constexpr MouseHover& hover() noexcept { return mHover; }
    constexpr MouseWheel& wheel() noexcept { return mWheel; }
    constexpr MouseButton& button() noexcept { return mButton; }

private:
    MousePos mPos;
    MouseHover mHover;
    MouseWheel mWheel;
    MouseButton mButton;
};

class AppState
{
public:

    constexpr const ClockState& clock() const noexcept { return mClock; }
    constexpr const WindowState& window() const noexcept { return mWindow; }
    constexpr const SystemState& system() const noexcept { return mSystem; }
    constexpr const KeyboardState& keyboard() const noexcept { return mKeys; }
    constexpr const MouseState& mouse() const noexcept { return mMouse; }

    constexpr ClockState& clock() noexcept { return mClock; }
    constexpr WindowState& window() noexcept { return mWindow; }
    constexpr SystemState& system() noexcept { return mSystem; }
    constexpr KeyboardState& keyboard() noexcept { return mKeys; }
    constexpr MouseState& mouse() noexcept { return mMouse; }

private:

    ClockState mClock;
    WindowState mWindow;
    SystemState mSystem;
    KeyboardState mKeys;
    MouseState mMouse;
};

class Events
{
public:

    bool pump_events();

    constexpr const AppState& get_state() const noexcept { return mState; }
    constexpr AppState& get_state() noexcept { return mState; }

protected:

    void reset_logical_events(double delta) noexcept;
private:
    Stopwatch mClock;
    AppState mState;
};