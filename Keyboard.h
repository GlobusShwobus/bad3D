#ifndef INPUT_H_
#define INPUT_H_

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

private:

	void handle_mouse_messages(UINT msg, WPARAM wParam, LPARAM lParam)noexcept
	{
		switch (msg)
		{
		case WM_KEYDOWN:
			m_keys[wParam] = true;
			break;
		case WM_KEYUP:
			m_keys[wParam] = false;
			break;
		}
	}
private:
	bool m_keys[KEY_BUFFER_SIZE];
};
#endif