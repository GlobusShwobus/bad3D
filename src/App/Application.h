#pragma once

#include <memory>
#include <string>

#include <d3d12.h>
#include <wrl/client.h>

#include "App/badWin32.h"
#include "App/IGame.h"
#include "App/AppState.h"
#include "D3D12/CommandQueue.h"
#include "D3D12/SwapChain.h"
#include "Tools/ViewPtr.h"

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

	void initialise(
		std::wstring window_name,
		HINSTANCE hInstance,
		UINT x,
		UINT y,
		UINT width,
		UINT height,
		HICON hIcon = nullptr,
		HICON hIconSm = nullptr,
		HCURSOR hCursor = nullptr
	);
	void    shutdown();

	void flush();

	ID3D12Device4* get_device() const noexcept;
	HWND           get_hwnd() const noexcept;
	SwapChain*     get_swap_chain() const noexcept;
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
	std::unique_ptr<SwapChain>       mSwapChain = nullptr;

	AppState mState;

	ViewPtr<IGame> mGame;

	bool mInitialised = false;
	bool mRunning = false;
};
