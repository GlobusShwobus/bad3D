#ifndef SYSWINDOW_H_
#define SYSWINDOW_H_

#include <windows.h>
#include <windowsx.h>

#include "IWindow.h"
#include "Keyboard.h"
#include "Mouse.h"
#include <stdexcept>
#include "IRenderWindowSync.h"

class Window : public InterfaceWindow<Window>
{
public:
	Window(HINSTANCE hInstance, const std::wstring& class_name, Keyboard& g_Keyboard, Mouse& g_mouse)
		:m_kb(g_Keyboard), m_mouse(g_mouse)
	{
		// globals (later make better constructor)
		static constexpr LONG g_width = 1280;
		static constexpr LONG g_height = 720;

		// try to register this window using interface API
		if (!register_window(hInstance, class_name))
			throw std::runtime_error("failed register");

		// get system screen dims
		int screenWidth = ::GetSystemMetrics(SM_CXSCREEN);
		int screenHeight = ::GetSystemMetrics(SM_CYSCREEN);

		// pass in the desired client area of the window and AdjustWindowRect will add to it the border pixel sizes
		// because CreateWindowEX wants the full size of the window, not just client area
		RECT windowRect = { 0, 0, g_width, g_height };
		::AdjustWindowRect(&windowRect, WS_OVERLAPPEDWINDOW, FALSE);

		int windowWidth = windowRect.right - windowRect.left;
		int windowHeight = windowRect.bottom - windowRect.top;

		// Center the window within the screen. Clamp to 0, 0 for the top-left corner.
		int windowX = std::max<int>(0, (screenWidth - windowWidth) / 2);
		int windowY = std::max<int>(0, (screenHeight - windowHeight) / 2);

		// try to init this window using interface API
		if (!initialize_window(class_name, hInstance, L"Main Window", WS_OVERLAPPEDWINDOW, NULL, windowX,windowY, windowWidth, windowHeight, NULL, NULL))
			throw std::runtime_error("failed create");
		
		m_width = windowWidth;
		m_height = windowHeight;
	}
	Window(const Window&) = delete;
	Window& operator=(const Window&) = delete;
	// TODO move construct

	LRESULT handle_message(UINT uMsg, WPARAM wParam, LPARAM lParam) override
	{
		// window must also update the mouse and keyboard events
		m_mouse.handle_mouse_messages(uMsg, wParam, lParam, m_hwnd);
		m_kb.handle_mouse_messages(uMsg, wParam, lParam);

		// set mouse capture, may be done in higher level code. will see
		if (m_mouse.button(Mouse::ButtonType::Left).pressed()) {
			SetCapture(m_hwnd);
		}
		else if(m_mouse.button(Mouse::ButtonType::Left).released()) 
		{
			if (GetCapture() == m_hwnd) {
				ReleaseCapture();
			}
		}

		// handle events
		switch (uMsg)
		{
		case WM_DESTROY:
			PostQuitMessage(0);
			return 0;
		case WM_PAINT:
		{
			PAINTSTRUCT ps;
			BeginPaint(m_hwnd, &ps);
			EndPaint(m_hwnd, &ps);
		}
		return 0;

		// on window resize get the width and height to the client area. alternative is width / height = LOPARAM / HIPARAM (lParam) but this gives entire size
		case WM_SIZE: 
		
			if (wParam != SIZE_MINIMIZED)
			{
				RECT clientRect;
				GetClientRect(m_hwnd, &clientRect);

				m_width = clientRect.right - clientRect.left;
				m_height = clientRect.bottom - clientRect.top;

				if (m_renderer_sync) m_renderer_sync->on_resize(get_width(), get_height());
			}
			return 0;

		// full screen toggle must be a special case becasue on messing with the window, something deep in win32 gets messed up.
		// currently i think the best is to approach it as a special case, hence add the event into the main event handle and not on physical key down / up
		case WM_KEYDOWN:

			if (wParam == VK_F11)
			{
				on_fullscreen_toggle();
			}

			return 0;

		default:
			break; // default breaks switch and goes to DefWindowProc
		}

		return DefWindowProc(m_hwnd, uMsg, wParam, lParam);
	}

	void show_window(int nCmdShow)
	{
		ShowWindow(m_hwnd, nCmdShow);
	}

	void end_frame(float dt)
	{
		m_mouse.end_frame(dt);
	}

	inline uint32_t get_width() const noexcept { return m_width; }
	inline uint32_t get_height() const noexcept { return m_height; }
	HWND window() const noexcept { return m_hwnd; }

	void sync_to_renderer_interface(InterfaceRenderWindowSync* sync) { m_renderer_sync = sync; }

private:

	void on_fullscreen_toggle()
	{
		m_isFullscreen = !m_isFullscreen;

		if (m_isFullscreen) // Switching to fullscreen.
		{
			// Store the current window dimensions so they can be restored 
			// when switching out of fullscreen state.
			::GetWindowRect(m_hwnd, &m_saveOnFullscreen);

			// Set the window style to a borderless window so the client area fills
			// the entire screen
			UINT windowStyle = WS_OVERLAPPEDWINDOW & ~(WS_CAPTION | WS_SYSMENU | WS_THICKFRAME | WS_MINIMIZEBOX | WS_MAXIMIZEBOX);
			::SetWindowLongPtrA(m_hwnd, GWL_STYLE, windowStyle);

			// Query the name of the nearest display device for the window.
			// This is required to set the fullscreen dimensions of the window
			// when using a multi-monitor setup.
			HMONITOR hMonitor = ::MonitorFromWindow(m_hwnd, MONITOR_DEFAULTTONEAREST);
			MONITORINFOEX monitorInfo = {};
			monitorInfo.cbSize = sizeof(MONITORINFOEX);
			::GetMonitorInfo(hMonitor, &monitorInfo);

			::SetWindowPos(m_hwnd, HWND_TOP,
				monitorInfo.rcMonitor.left,
				monitorInfo.rcMonitor.top,
				monitorInfo.rcMonitor.right - monitorInfo.rcMonitor.left,
				monitorInfo.rcMonitor.bottom - monitorInfo.rcMonitor.top,
				SWP_FRAMECHANGED | SWP_NOACTIVATE);

			::ShowWindow(m_hwnd, SW_MAXIMIZE);
		}
		else
		{
			// Restore all the window decorators.
			::SetWindowLongPtrA(m_hwnd, GWL_STYLE, WS_OVERLAPPEDWINDOW);

			// THIS IS ACTUALLY BROKEN CURRENTLY, YOU CAN GO FULLSCREEN BUT NOT BACK TO NORMIE SCREEN
			::SetWindowPos(m_hwnd, HWND_NOTOPMOST,
				m_saveOnFullscreen.left,
				m_saveOnFullscreen.top,
				m_saveOnFullscreen.right - m_saveOnFullscreen.left,
				m_saveOnFullscreen.bottom - m_saveOnFullscreen.top,
				SWP_FRAMECHANGED | SWP_NOACTIVATE);

			::ShowWindow(m_hwnd, SW_NORMAL);
		}
	}

private:

	Keyboard& m_kb;
	Mouse& m_mouse;

	RECT m_saveOnFullscreen{};
	bool m_isFullscreen = false;
	uint32_t m_width = 0;
	uint32_t m_height = 0;

	InterfaceRenderWindowSync* m_renderer_sync = nullptr;
};
#endif