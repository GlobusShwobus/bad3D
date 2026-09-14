#pragma once

#include "KeyTypes.h"
class Application;

class Events
{
    friend class Application;

    struct TimeEvent
    {
        double delta;
        double total;
    };

    struct WindowResizeEvent
    {
        int width;
        int height;
    };

    struct KeyboardEvent
    {
        bool keys[256];
    };

    struct MouseEvent
    {
        int pos_x;
        int pos_y;
        bool buttons[static_cast<unsigned long long>(MouseButtonType::Count)];
        double hover_duration;
        int wheel_delta;
        float wheel_delta_normalized;
    };

public:
    constexpr const TimeEvent& Time() const noexcept { return mTimeEvent; }
    constexpr const WindowResizeEvent& WindowSize() const noexcept { return mResizeEvent; }
    constexpr const KeyboardEvent& Keyboard() const noexcept { return mKeyboard; }
    constexpr const MouseEvent& Mouse() const noexcept { return mMouse; }

protected:
    constexpr void SetTimeEvent(double delta) noexcept { mTimeEvent.delta = delta; mTimeEvent.total += delta; }
    constexpr void SetWindowResizeEvent(int w, int h)noexcept { mResizeEvent.width = w; mResizeEvent.height = h; }
    constexpr void SetKeyBoard(unsigned long long param, bool state) noexcept { mKeyboard.keys[param] = state; }
    constexpr void SetMousePosition(int x, int y) noexcept { mMouse.pos_x = x; mMouse.pos_y = y; }
    constexpr void SetMouseButton(MouseButtonType button, bool state) noexcept { mMouse.buttons[static_cast<unsigned long long>(button)] = state; }
    constexpr void UpdateMouseHover(double delta) noexcept { mMouse.hover_duration += delta; }
    constexpr void ResetMouseHover() noexcept { mMouse.hover_duration = 0.0f; }
    constexpr void SetMouseWheel(int delta, int unit_delta_value) noexcept { mMouse.wheel_delta = delta; mMouse.wheel_delta_normalized = delta / static_cast<float>(unit_delta_value); }
    constexpr void ResetMouseWheel() noexcept { mMouse.wheel_delta = 0; mMouse.wheel_delta_normalized = 0; }
private:
    TimeEvent mTimeEvent;
    WindowResizeEvent mResizeEvent;
    KeyboardEvent mKeyboard;
    MouseEvent mMouse;
};