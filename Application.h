#pragma once

#include "badWin32.h"
#include <d3d12.h>
#include <wrl/client.h>

#include <memory>
#include <string>

#include "CommandQueue.h"
#include "RenderWindow.h"
#include "ViewPtr.h"
#include "IGame.h"
#include "AppState.h"

struct AppWinDesc
{
	std::wstring window_name;
	DWORD window_style = 0;
	HINSTANCE hInstance = nullptr;

	HICON hIcon = nullptr;
	HICON hIconSm = nullptr;
	HCURSOR hCursor = nullptr;

	int x = 0;
	int y = 0;
	int cw = 0;
	int ch = 0;
};

class Application final
{
	Application() = default;
	Application(const Application&) = delete;
	Application& operator=(const Application&) = delete;
	Application(Application&&) = delete;
	Application& operator=(Application&&) = delete;

public:

	virtual ~Application();

	static Application& instance() noexcept
	{
		static Application sInstance;
		return sInstance;
	}

	void    initialise(AppWinDesc window_desc);
	void    shutdown();

	void flush();

	ID3D12Device4* get_device() const noexcept;
	HWND           get_hwnd() const noexcept;
	RenderWindow*  get_render_window() const noexcept;
	CommandQueue*  get_command_queue(D3D12_COMMAND_LIST_TYPE type) const noexcept;
	const AppState& get_state_manager() const noexcept { return mState; }

	inline void set_game(ViewPtr<IGame> game) noexcept { mGame = game; }
	inline void exit_loop() noexcept { mRunning = false; }

	void run();

protected:

	LRESULT on_message(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

	static LRESULT CALLBACK wnd_proc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
	{
		Application* self = nullptr;
		if (uMsg == WM_NCCREATE) {
			CREATESTRUCT* pCreate = (CREATESTRUCT*)lParam;
			self = (Application*)pCreate->lpCreateParams;
			SetWindowLongPtrW(hwnd, GWLP_USERDATA, (LONG_PTR)self);
			self->mHwnd = hwnd;
		}
		else {
			self = (Application*)GetWindowLongPtrW(hwnd, GWLP_USERDATA);
		}

		if (self)
			return self->on_message(hwnd, uMsg, wParam, lParam);

		return DefWindowProcW(hwnd, uMsg, wParam, lParam);
	}

	void update_other_events(double delta);
private:

	Microsoft::WRL::ComPtr<ID3D12Device4> mDevice = nullptr;

	std::unique_ptr<CommandQueue>   mDirectCommandQueue = nullptr;
	std::unique_ptr<CommandQueue>   mComputeCommandQueue = nullptr;
	std::unique_ptr<CommandQueue>   mCopyCommandQueue = nullptr;

	HWND mHwnd = nullptr;
	std::unique_ptr<RenderWindow>       mRenderWindow = nullptr;

	AppState mState;

	ViewPtr<IGame> mGame;

	bool mInitialised = false;
	bool mRunning = false;
};
