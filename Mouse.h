#ifndef MOUSE_H
#define MOUSE_H

#include <windowsx.h>

class Mouse
{   
    friend class Window;
public:

    class ButtonState
    {
        friend class Mouse;
    public:

        // says if the button is down or not: true == down, false == up
        inline bool down() const noexcept { return m_down; }

        // says if the button was clicked this exact frame: true == yes, false == no
        inline bool pressed() const noexcept { return m_down && !m_prev; }

        // says if the button was released this exact frame: true == yes, false == no
        inline bool released() const noexcept { return !m_down && m_prev; }

    private:
        bool m_down = false;
        bool m_prev = false;
    };

    enum class ButtonType :unsigned int
    {
        Left,
        Right,
        Middle,
        Count
    };

    // MOUSE POS
    inline int pos_x() const noexcept { return m_x; }
    inline int pos_y() const noexcept { return m_y; }

    // MOUSE BUTTON STATE
    const ButtonState& button(ButtonType b)const noexcept { return m_buttons[(unsigned int)b]; }

    // MOUSEE WHEEL
    inline int consume_wheel() noexcept 
    { 
        int delta = m_wheelDelta;
        m_wheelDelta = 0;
        return delta;
    }

    // MOUSE HOVER
    bool is_hovering(float threshold)const noexcept { return m_hoverTime >= threshold; }

private:
    void handle_mouse_messages(UINT msg, WPARAM wParam, LPARAM lParam, HWND hWnd) noexcept
    {
        switch (msg)
        {
        case WM_MOUSEMOVE:
            {
                int x = GET_X_LPARAM(lParam);
                int y = GET_Y_LPARAM(lParam);

                if (x != m_x || y != m_y) // because windows can generate WM_MOUSEMOVE even when mouse seems stationary
                    m_hoverTime = 0.0f;

                m_x = x;
                m_y = y;
            }
            break;

        case WM_LBUTTONDOWN:
            m_buttons[(unsigned int)ButtonType::Left].m_down = true;
            break;

        case WM_LBUTTONUP:
            m_buttons[(unsigned int)ButtonType::Left].m_down = false;
            break;

        case WM_RBUTTONDOWN:
            m_buttons[(unsigned int)ButtonType::Right].m_down = true;
            break;

        case WM_RBUTTONUP:
            m_buttons[(unsigned int)ButtonType::Right].m_down = false;
            break;

        case WM_MOUSEWHEEL:
            m_wheelDelta += GET_WHEEL_DELTA_WPARAM(wParam);
            break;
        default:
            break;
        }
    }

    // this should be called with renderers end frame
    void end_frame(float dt) noexcept
    {
        // update every buttons state
        for (unsigned int i = 0; i < (unsigned int)ButtonType::Count; i++)
            m_buttons[i].m_prev = m_buttons[i].m_down;

        // update hover time
        m_hoverTime += dt;
    }

private:
    int m_x = 0;
    int m_y = 0;

    ButtonState m_buttons[(unsigned int)ButtonType::Count];

    int m_wheelDelta = 0;

    float m_hoverTime = 0.0f;
};
#endif