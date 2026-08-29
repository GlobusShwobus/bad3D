#pragma once

#include "badWin32.h"
#include "badDirectX.h"
#include <wrl/client.h>

#include <memory>
#include <string>

#include "CommandQueue.h"
#include "RenderWindow.h"
#include "ViewPtr.h"
#include "IGame.h"

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

	constexpr ViewPtr<ID3D12Device4> get_device() const noexcept { return mDevice.Get(); }
	constexpr ViewPtr<HWND__>        get_hwnd() const noexcept { return mHwnd; }
	constexpr ViewPtr<RenderWindow>  get_render_window() const noexcept { return mRenderWindow.get(); }
	inline ViewPtr<CommandQueue>     get_command_queue(D3D12_COMMAND_LIST_TYPE type) const noexcept
	{
		ViewPtr<CommandQueue> p = nullptr;

		if (type == D3D12_COMMAND_LIST_TYPE_DIRECT)
			p = mDirectCommandQueue.get();
		else if (type == D3D12_COMMAND_LIST_TYPE_COMPUTE)
			p = mComputeCommandQueue.get();
		else if (type == D3D12_COMMAND_LIST_TYPE_COPY)
			p = mCopyCommandQueue.get();

		return p;
	}

	inline void set_game(ViewPtr<IGame> game) noexcept { mGame = game; }
	void run();

protected:

	Microsoft::WRL::ComPtr<IDXGIAdapter4> find_adapter(ViewPtr<IDXGIFactory4> factory, bool use_warp);

	void init_device(ViewPtr<IDXGIFactory4> factory4, ViewPtr<IDXGIAdapter4> adapter4);
	void init_command_queues();
	void init_HWND(const AppWinDesc& window_desc);
	void init_swap_chain(ViewPtr<IDXGIFactory4> factory4, DWORD window_style);

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
private:

	Microsoft::WRL::ComPtr<ID3D12Device4> mDevice = nullptr;

	std::unique_ptr<CommandQueue>   mDirectCommandQueue = nullptr;
	std::unique_ptr<CommandQueue>   mComputeCommandQueue = nullptr;
	std::unique_ptr<CommandQueue>   mCopyCommandQueue = nullptr;

	HWND mHwnd = nullptr;
	std::unique_ptr<RenderWindow>       mRenderWindow = nullptr;

	ViewPtr<IGame> mGame;

	bool dx12_initalised = false;
};
