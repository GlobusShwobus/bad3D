#pragma once

enum class MouseButtonType : unsigned int
{
    Left,
    Right,
    Middle,
    Count
};

class SystemState
{
public:
    // getters, viewed globally
    constexpr double DeltaTime() const noexcept { return mDeltaAge; }
    constexpr double Age() const noexcept { return mTotalAge; }
    constexpr bool WindowResizeEvent() const noexcept { return mWindowResizeEvent; }
    constexpr int  WindowResizeWidth() const noexcept { return mResizeWidth; }
    constexpr int  WindowResizeHeight() const noexcept { return mResizeHeight; }
    constexpr bool SystemQuitEvent() const noexcept { return mQuitEvent; }

    // setters, only seen in app
    constexpr void UpdateAge(double delta) noexcept { mTotalAge += delta; mDeltaAge = delta; }
    constexpr void SetResizeEvent(int w, int h) noexcept { mWindowResizeEvent = true; mResizeWidth = w; mResizeHeight = h; }
    constexpr void ResetResizeEvent() noexcept { mWindowResizeEvent = false; }
    constexpr bool SetQuitEvent() noexcept { return mQuitEvent = true; }

private:
    double mDeltaAge = 0.0;
    double mTotalAge = 0.0;

    bool mWindowResizeEvent = false;
    int mResizeWidth = 0;
    int mResizeHeight = 0;

    bool mQuitEvent = false;
};

class KeyboardState
{
public:
    // getters, viewed globally
    const bool* GetKeys() const noexcept { return mKeys; }
    const bool IsKeyPressed(unsigned char key) const { return mKeys[key]; }

    // setters, only seen in app
    constexpr void SetUp(unsigned long long param) noexcept { mKeys[param] = false; }
    constexpr void SetDown(unsigned long long param) noexcept { mKeys[param] = true; }

private:
    bool mKeys[256] = { false };
};

class MouseState
{
public:
    // getters
    constexpr int PosX() const noexcept { return mPosX; }
    constexpr int PosY() const noexcept { return mPosY; }
    constexpr bool ButtonState(MouseButtonType button) const noexcept { return mButtons[(unsigned long long)(button)]; }
    constexpr double HoverDuration() const noexcept { return mHoverDuration; }
    constexpr int WheelDeltaUnits() const noexcept { return mWheelDelta; }
    constexpr float WheelDeltaNormalized() const noexcept { return mWheelDeltaNormalized; }

    // setters
    constexpr void SetPosition(int x, int y) noexcept { mPosX = x; mPosY = y; }
    constexpr void SetButtonUp(MouseButtonType button) noexcept { mButtons[(unsigned long long)(button)] = false; }
    constexpr void SetButtonDown(MouseButtonType button) noexcept { mButtons[(unsigned long long)(button)] = true; }
    constexpr void UpdateHoverDuration(double delta) noexcept { mHoverDuration += delta; }
    constexpr void ResetHoverDuration() noexcept { mHoverDuration = 0.0f; }
    constexpr void SetWheelDelta(int delta, int unit_delta_value) noexcept { mWheelDelta = delta; mWheelDeltaNormalized = delta / static_cast<float>(unit_delta_value); }
    constexpr void ResetWheelDelta() noexcept { mWheelDelta = 0; mWheelDeltaNormalized = 0; }
private:
    int mPosX = 0;
    int mPosY = 0;
    bool mButtons[(unsigned long long)(MouseButtonType::Count)] = { false };
    double mHoverDuration = 0.0;
    int mWheelDelta = 0;
    float mWheelDeltaNormalized = 0.0f;
};

class AppState
{
public:

    constexpr const SystemState& System() const noexcept { return mSysEvents; }
    constexpr const KeyboardState& Keyboard() const noexcept { return mKeyEvents; }
    constexpr const MouseState& Mouse() const noexcept { return mMouse; }

    constexpr SystemState& System() noexcept { return mSysEvents; }
    constexpr KeyboardState& Keyboard() noexcept { return mKeyEvents; }
    constexpr MouseState& Mouse() noexcept { return mMouse; }

private:
    SystemState mSysEvents;
    KeyboardState mKeyEvents;
    MouseState mMouse;
};