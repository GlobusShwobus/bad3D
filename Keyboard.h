#pragma once
#include "WIN32_CORE.h"

class Keyboard
{
	static constexpr unsigned int KEY_BUFFER_SIZE = 256;
public:

	Keyboard()
	{
		for (int i = 0; i < KEY_BUFFER_SIZE; i++)
			mKeys[i] = false;
	}
	virtual ~Keyboard() = default;

	const bool* get_keys()const noexcept { return mKeys; }

	constexpr void resolve_message(UINT uMsg, WPARAM wParam, LPARAM lParam)
	{
		switch (uMsg)
		{
		case WM_SYSKEYDOWN:
		case WM_KEYDOWN:
			mKeys[wParam] = true;
			break;

		case WM_SYSKEYUP:
		case WM_KEYUP:
			mKeys[wParam] = false;
			break;
		default:
			break;
		}
	}

private:
	bool mKeys[KEY_BUFFER_SIZE];
};