#pragma once

#include "Tools/Stopwatch.h"
#include "App/InputCodes.h"

class ClockState
{
public:

    constexpr ClockState() noexcept
        :mDeltaAge(0.0), mTotalAge(0.0)
    { }

    constexpr double delta() const noexcept { 
        return mDeltaAge; 
    }

    constexpr double age() const noexcept { 
        return mTotalAge; 
    }

    constexpr void update(double delta) noexcept {
        mTotalAge += delta; 
        mDeltaAge = delta; 
    }

private:
    double mDeltaAge;
    double mTotalAge;
};

class WindowState
{
public:

    constexpr WindowState() noexcept
        :mWidth(0), mHeight(0), mEvent(false)
    {}

    constexpr bool is_resized() const noexcept {
        return mEvent; 
    }

    constexpr int width() const noexcept { 
        return mWidth; 
    }
    constexpr int height() const noexcept {
        return mHeight; 
    }

    constexpr void resize(int w, int h) noexcept { 
        mEvent = true;
        mWidth = w; 
        mHeight = h;
    }

    constexpr void reset() noexcept { 
        mEvent = false; 
    }

private:
    int mWidth;
    int mHeight;
    bool mEvent;
};

class SystemState
{
public:

    constexpr SystemState() noexcept
        :mSysQuit(false)
    {}

    constexpr bool is_system_quit() const noexcept { 
        return mSysQuit; 
    }

    constexpr bool exit() noexcept { 
        return mSysQuit = true; 
    }

private:
    bool mSysQuit;
};

class KeyboardState
{
public:

    constexpr KeyboardState() noexcept
        :mKeys{ false }
    {}

    const bool* keys() const noexcept { 
        return mKeys; 
    }

    const bool is_key_pressed(unsigned char key) const noexcept { 
        return mKeys[key]; 
    }

    constexpr void set_up(unsigned long long param) noexcept {
        mKeys[param] = false; 
    }

    constexpr void set_down(unsigned long long param) noexcept { 
        mKeys[param] = true; 
    }

private:
    bool mKeys[256];
};

class MousePos
{
public:

    constexpr MousePos() noexcept
        :mPosX(0), mPosY(0)
    { }

    constexpr int x() const noexcept {
        return mPosX; 
    }

    constexpr int y() const noexcept {
        return mPosY; 
    }

    constexpr void set(int x, int y) noexcept { 
        mPosX = x; mPosY = y; 
    }

    constexpr void shift(int deltaX, int deltaY) noexcept {
        mPosX += deltaX; mPosY += deltaY;
    }

private:
    int mPosX;
    int mPosY;
};

class MouseHover
{
public:

    constexpr MouseHover() noexcept
        :mHoverDuration(0.0)
    {}

    constexpr double duration() const noexcept { 
        return mHoverDuration; 
    }

    constexpr void update(double delta) noexcept { 
        mHoverDuration += delta;
    }

    constexpr void reset() noexcept {
        mHoverDuration = 0.0;
    }

private:
    double mHoverDuration;
};

class MouseWheel
{
public:
    constexpr MouseWheel() noexcept
        :mWheelDelta(0), mWheelDeltaNormalized(0.0f)
    {}
    
    constexpr int units() const noexcept { 
        return mWheelDelta; 
    }

    constexpr float normalized() const noexcept { 
        return mWheelDeltaNormalized;
    }

    constexpr void set(int delta, int wheel_delta_per_indent) noexcept {
        mWheelDelta = delta; mWheelDeltaNormalized = delta / static_cast<float>(wheel_delta_per_indent);
    }

    constexpr void reset() noexcept {
        mWheelDelta = 0;
        mWheelDeltaNormalized = 0; 
    }

private:

    int mWheelDelta;
    float mWheelDeltaNormalized;
};

class MouseButton
{
public:

    constexpr MouseButton() noexcept
        :mButtons{ false } 
    {}

    constexpr bool is_down(MouseButtonType button) const noexcept {
        return mButtons[(unsigned long long)(button)] == true;
    }

    constexpr void set_up(MouseButtonType button) noexcept { 
        mButtons[(unsigned long long)(button)] = false; 
    }

    constexpr void set_down(MouseButtonType button) noexcept { 
        mButtons[(unsigned long long)(button)] = true; 
    }

private:
    bool mButtons[(unsigned long long)(MouseButtonType::Count)];
};

class MouseState
{
public:

    constexpr MouseState() noexcept = default;

    constexpr const MousePos& position() const noexcept { 
        return mPos; 
    }
    constexpr MousePos& position() noexcept {
        return mPos;
    }

    constexpr const MouseHover& hover() const noexcept {
        return mHover; 
    }
    constexpr MouseHover& hover() noexcept {
        return mHover;
    }

    constexpr const MouseWheel& wheel() const noexcept { 
        return mWheel; 
    }
    constexpr MouseWheel& wheel() noexcept {
        return mWheel;
    }

    constexpr const MouseButton& button() const noexcept {
        return mButton;
    }
    constexpr MouseButton& button() noexcept {
        return mButton;
    }

private:
    MousePos mPos;
    MouseHover mHover;
    MouseWheel mWheel;
    MouseButton mButton;
};

class AppState
{
    // the question might be why... because fuck you it works sexily
public:

    constexpr AppState() noexcept = default;

    constexpr const ClockState& clock() const noexcept { 
        return mClock; 
    }
    constexpr ClockState& clock() noexcept {
        return mClock;
    }

    constexpr const WindowState& window() const noexcept {
        return mWindow; 
    }
    constexpr WindowState& window() noexcept {
        return mWindow;
    }

    constexpr const SystemState& system() const noexcept { 
        return mSystem;
    }
    constexpr SystemState& system() noexcept {
        return mSystem;
    }

    constexpr const KeyboardState& keyboard() const noexcept {
        return mKeys; 
    }
    constexpr KeyboardState& keyboard() noexcept {
        return mKeys;
    }

    constexpr const MouseState& mouse() const noexcept {
        return mMouse;  
    }
    constexpr MouseState& mouse() noexcept {
        return mMouse;  
    }

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

    bool pump_events() noexcept;

    constexpr const AppState& get_state() const noexcept {
        return mState; 
    }

    constexpr AppState& get_state() noexcept {
        return mState; 
    }

protected:

    void reset_logical_events(double delta) noexcept
    {
        mState.clock().update(delta);
        mState.window().reset();
        mState.mouse().hover().update(delta);
        mState.mouse().wheel().reset();
    }

private:
    Stopwatch mClock;
    AppState mState;
};