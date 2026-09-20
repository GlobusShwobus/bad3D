#pragma once

#include <string>

#include <d3d12.h>
#include <dxgi1_6.h>
#include <wrl/client.h>

#include "App/AppState.h"
#include "App/GraphicsDevice.h"
#include "D3D12/DescriptorHeap.h"

struct RENDER_WINDOW_DESC
{
	std::wstring window_name;             // must be set
	HINSTANCE hInstance = nullptr;        // must be set
	UINT x = 0;
	UINT y = 0;
	UINT width = 1u;
	UINT height = 1u;
	HICON hIcon = nullptr;		          // can be left as null
	HICON hIconSm = nullptr;	          // can be left as null
	HCURSOR hCursor = nullptr;	          // can be left as null
};

class RenderWindow final
{
	struct ScreenToggle
	{
		RECT   window_rect{ 0,0,0,0 };
		UINT   window_style = 0;
		bool   is_fullscreen = false;

		void toggle_window_to(HWND hwnd, bool mode);
	};

public:

	static constexpr UINT BACK_BUFFER_COUNT = 3u;

	RenderWindow(
		AppState& events,
		GraphicsDevice& device,
		RENDER_WINDOW_DESC desc
	);

	RenderWindow(const RenderWindow&) = delete;
	RenderWindow& operator=(const RenderWindow&) = delete;

	RenderWindow(RenderWindow&&) = delete;
	RenderWindow& operator=(RenderWindow&&) = delete;

	~RenderWindow()
	{
		mQueue.flush_execution();
		if (mHwnd)
			::DestroyWindow(mHwnd);
	}

	void set_clear_color(float r, float g, float b, float a);

	CommandList get_command_list();

	void begin();

	void submit_work(CommandList&& list);

	void present();

	void resize(UINT client_width, UINT client_height);
	void toggle_fullscreen(bool fullscreen);

	ID3D12Resource* get_buffer() const {
		return mBackBuffers[mCurrentBufferIndex].Get();
	}
	D3D12_CPU_DESCRIPTOR_HANDLE get_buffer_desc() const { return mDescHeap.descriptor_at(mCurrentBufferIndex); }

	constexpr UINT              get_buffer_index()  const noexcept { return mCurrentBufferIndex; }
	constexpr UINT              get_buffer_count()  const noexcept { return BACK_BUFFER_COUNT; }
	constexpr UINT              get_buffer_width()  const noexcept { return mBufferWidth; }
	constexpr UINT              get_buffer_height() const noexcept { return mBufferHeight; }
	constexpr UINT64            get_buffer_signal() const noexcept { return mBackBufferCompletionTrackers[mCurrentBufferIndex]; }

protected:

	void update_back_buffers();

	LRESULT on_message(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

	static LRESULT CALLBACK wnd_proc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
	{
		RenderWindow* self = nullptr;
		if (uMsg == WM_NCCREATE) {
			CREATESTRUCT* pCreate = (CREATESTRUCT*)lParam;
			self = (RenderWindow*)pCreate->lpCreateParams;
			SetWindowLongPtrW(hwnd, GWLP_USERDATA, (LONG_PTR)self);
			self->mHwnd = hwnd;
		}
		else {
			self = (RenderWindow*)GetWindowLongPtrW(hwnd, GWLP_USERDATA);
		}

		if (self)
			return self->on_message(hwnd, uMsg, wParam, lParam);

		return DefWindowProcW(hwnd, uMsg, wParam, lParam);
	}

private:

	// window / swapchain stuff
	HWND                                    mHwnd = nullptr;
	Microsoft::WRL::ComPtr<IDXGISwapChain4> mSwapChain = nullptr;
	DescriptorHeap                          mDescHeap;
	Microsoft::WRL::ComPtr<ID3D12Resource>  mBackBuffers[BACK_BUFFER_COUNT];
	UINT64                                  mBackBufferCompletionTrackers[BACK_BUFFER_COUNT];

	// borrowed stuff from device
	ViewPtr<ID3D12Device4>                  mDevice = nullptr;
	CommandQueue& mQueue;
	AppState& mState;

	// buffer info
	UINT mCurrentBufferIndex = 0;
	UINT mBufferWidth = 0;
	UINT mBufferHeight = 0;

	// stuffz
	ScreenToggle       mScreenToggle;

	bool mIsVSync = false;
	bool mIsTearingSupported = false;
	bool mInitialised = false;

	FLOAT mClearColor[4] = { 0.0f, 0.0f, 0.0f, 1.0f };
};