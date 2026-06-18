#ifndef IWINDOW_H_
#define IWINDOW_H_

#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#include <string>

// turn off windows min / max so they don't conflict with anything
#if defined(min)
#undef min
#endif

#if defined(max)
#undef max
#endif

template <typename DERIVED_TYPE>
class InterfaceWindow
{
protected:
	InterfaceWindow() :m_hwnd(nullptr) {}
	
	virtual ~InterfaceWindow() noexcept
	{
		reset();
	}

	// windows procedure that is called by the kernel. internally delegates runtime logic events to virtual handle_message function
	static LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
	{
		DERIVED_TYPE* pThis = nullptr;

		if (uMsg == WM_NCCREATE)
		{
			CREATESTRUCT* pCreate = (CREATESTRUCT*)lParam;
			pThis = (DERIVED_TYPE*)pCreate->lpCreateParams;
			SetWindowLongPtr(hwnd, GWLP_USERDATA, (LONG_PTR)pThis);

			pThis->m_hwnd = hwnd;
		}
		else
			pThis = (DERIVED_TYPE*)GetWindowLongPtr(hwnd, GWLP_USERDATA);

		if (pThis)
			return pThis->handle_message(uMsg, wParam, lParam);
		else
			return DefWindowProc(hwnd, uMsg, wParam, lParam); // throw might be better, this should not happen in my usecase so far
	}

	// user defined message handling
	virtual LRESULT handle_message(UINT uMsg, WPARAM wParam, LPARAM lParam) = 0;

	// window register that works with anything that inherits from the interface window. if the function returns false, it failed to register (likely duplicate)
	bool register_window(HINSTANCE hInstance, const std::wstring& class_name)
	{
		WNDCLASSEX wc = { 0 };

		wc.cbSize = sizeof(wc);
		wc.style = CS_HREDRAW | CS_VREDRAW;

		wc.lpfnWndProc = InterfaceWindow<DERIVED_TYPE>::WindowProc; // binding a static protocol with a non static obj where *this is the state handler (genious windows design)
		wc.hInstance = GetModuleHandle(NULL);      // ask for the main window, which should be default
		wc.lpszClassName = class_name.c_str();           // hardcoded class name, can also be a global variable or a static constexpr in the class

		wc.cbClsExtra = 0;
		wc.cbWndExtra = 0;

		wc.hIcon = nullptr;							// null so far
		wc.hCursor = nullptr;						// null so far
		wc.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);					// null so far
		wc.lpszMenuName = nullptr;					// null so far
		wc.hIconSm = nullptr;						// null so far

		ATOM atom = RegisterClassExW(&wc);

		return atom != 0 ? true : false;
	}

	// initalizes the window with given params
	bool initialize_window(
		const std::wstring& class_name,
		HINSTANCE hInstance,
		PCWSTR lpWindowName,
		DWORD dwStyle,
		DWORD dwExStyle = 0,
		int x = CW_USEDEFAULT,
		int y = CW_USEDEFAULT,
		int nWidth = CW_USEDEFAULT,
		int nHeight = CW_USEDEFAULT,
		HWND hWndParent = 0,
		HMENU hMenu = 0
	)
	{
		// don't recreate if already have a window
		if (m_hwnd != nullptr)
			return false;

		m_hwnd = CreateWindowExW(
			dwExStyle,
			class_name.c_str(),
			lpWindowName,
			dwStyle,
			x, y,
			nWidth,
			nHeight,
			hWndParent,
			hMenu,
			hInstance,
			this  // Pass 'this' as the creation parameter
		);

		return m_hwnd != nullptr;
	}

	void reset() noexcept
	{
		if (m_hwnd != NULL)
		{
			DestroyWindow(m_hwnd);
			m_hwnd = nullptr;
		}
	}

	HWND m_hwnd;
};
#endif