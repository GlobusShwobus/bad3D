#pragma once

#include "badWin32.h"
#include <array>

//todo: wheel class

#define GET_MOUSE_X_POS(lp) ((int)(short)LOWORD(lParam))
#define GET_MOUSE_Y_POS(lp) ((int)(short)HIWORD(lParam))

class Mouse;

enum class MouseButtonType : unsigned int
{
    Left,
    Right,
    Middle,
    Count
};

class MouseButton
{
    friend class Mouse;

public:

    MouseButton() = default;

    // says if the button is down or not: true == down, false == up
    constexpr bool is_down() const noexcept { return mDown; }

    // says if the button was clicked this exact frame: true == yes, false == no
    constexpr bool is_pressed() const noexcept { return mDown && !mPrev; }

    // says if the button was released this exact frame: true == yes, false == no
    constexpr bool is_released() const noexcept { return !mDown && mPrev; }

protected:

    constexpr void set_down() noexcept            { mDown = true; }
    constexpr void set_up() noexcept              { mDown = false; }
    constexpr void set_prev_to_current() noexcept { mPrev = mDown; }

private:
    bool mDown = false;
    bool mPrev = false;
};

class Mouse
{   
    using Buttons = std::array<MouseButton, static_cast<unsigned int>(MouseButtonType::Count)>;

public:

    Mouse() = default;
    virtual ~Mouse() = default;

    // MOUSE POS
    constexpr int get_pos_x() const noexcept { return mX; }
    constexpr int get_pos_y() const noexcept { return mY; }

    // MOUSE HOVER
    constexpr float get_hovering_time() const noexcept { return mHoverTime; }

    // MOUSE BUTTON STATE
    constexpr const MouseButton& get_button(MouseButtonType type) const noexcept { return mButtons[static_cast<unsigned int>(type)]; }

    // MOUSEE WHEEL
    constexpr int get_wheel_delta() noexcept 
    { 
        int delta = mWheel;
        mWheel = 0;
        return delta;
    }

    constexpr void resolve_message(UINT msg, WPARAM wParam, LPARAM lParam) noexcept
    {
        switch (msg)
        {
        case WM_MOUSEMOVE:
        {
            int nx = GET_MOUSE_X_POS(lParam);
            int ny = GET_MOUSE_Y_POS(lParam);

            if (nx != mX || ny != mY) // because windows can generate WM_MOUSEMOVE even when mouse seems stationary
                mHoverTime = 0.0f;

            mX = nx;
            mY = ny;
        }
        break;

        case WM_LBUTTONDOWN:
            mButtons[static_cast<unsigned int>(MouseButtonType::Left)].set_down();
            break;

        case WM_LBUTTONUP:
            mButtons[static_cast<unsigned int>(MouseButtonType::Left)].set_up();
            break;

        case WM_RBUTTONDOWN:
            mButtons[static_cast<unsigned int>(MouseButtonType::Right)].set_down();
            break;

        case WM_RBUTTONUP:
            mButtons[static_cast<unsigned int>(MouseButtonType::Right)].set_up();
            break;

        case WM_MOUSEWHEEL:
            mWheel += GET_WHEEL_DELTA_WPARAM(wParam);
            break;
        default:
            break;
        }
    }

    constexpr void update_mouse_buttons(float dt) noexcept
    {
        // update every buttons state
        for (auto& e : mButtons)
            e.set_prev_to_current();

        // update hover time
        mHoverTime += dt;
    }

private:
    int mX = 0;
    int mY = 0;

    Buttons mButtons;

    int mWheel = 0;

    float mHoverTime = 0.0f;
};