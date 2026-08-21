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

class Application final
{
	// windows advanced rasterization protocol
	static constexpr bool using_WARP_adapter = false;

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

	void    initialise(const std::wstring& title, UINT x, UINT y, UINT client_width, UINT client_height, DWORD window_style, HINSTANCE hInstance);
	void    shutdown();

	void flush();

	constexpr ViewPtr<ID3D12Device4> get_device() const noexcept { return mDevice.Get(); }
	constexpr ViewPtr<RenderWindow>  get_window() const noexcept { return mRenderWindow.get(); }
	constexpr ViewPtr<CommandQueue>  get_command_queue(D3D12_COMMAND_LIST_TYPE type) const noexcept
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

	constexpr void set_game(ViewPtr<IGame> game) noexcept { mGame = game; }
	void run();

protected:

	Microsoft::WRL::ComPtr<IDXGIAdapter4> find_adapter(ViewPtr<IDXGIFactory4> factory, bool use_warp);
	void initialise_dx12();
	void initialise_render_window(const std::wstring& title, UINT x, UINT y, UINT w, UINT h, DWORD window_style, HINSTANCE hInstance);
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

	Microsoft::WRL::ComPtr<IDXGIFactory4> mFactory = nullptr;
	Microsoft::WRL::ComPtr<ID3D12Device4> mDevice = nullptr;

	std::unique_ptr<CommandQueue>   mDirectCommandQueue = nullptr;
	std::unique_ptr<CommandQueue>   mComputeCommandQueue = nullptr;
	std::unique_ptr<CommandQueue>   mCopyCommandQueue = nullptr;

	HWND mHwnd = nullptr;
	std::unique_ptr<RenderWindow>       mRenderWindow = nullptr;

	ViewPtr<IGame> mGame;

	bool dx12_initalised = false;
};
