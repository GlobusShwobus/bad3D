#pragma once
#include "WIN32_CORE.h"
#define KEY_BUFFER_SIZE 256
class Keyboard
{
	friend class Window;
public:

	Keyboard()
	{
		for (int i = 0; i < KEY_BUFFER_SIZE; i++)
			m_keys[i] = false;
	}

	const bool* get_keys()const noexcept { return m_keys; }

	constexpr void resolve_message(UINT uMsg, WPARAM wParam, LPARAM lParam)
	{
		switch (uMsg)
		{
		case WM_SYSKEYDOWN:
		case WM_KEYDOWN:
			m_keys[wParam] = true;
			break;

		case WM_SYSKEYUP:
		case WM_KEYUP:
			m_keys[wParam] = false;
			break;
		default:
			break;
		}
	}

private:
	bool m_keys[KEY_BUFFER_SIZE];
};