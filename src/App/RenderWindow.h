#pragma once

#include <memory>
#include <string>
#include <array>

#include "App/badWin32.h" // FOR LEAN AND MEAN GO FIRST

#include <d3d12.h>
#include <dxgi1_6.h>
#include <wrl/client.h>

#include "App/Events.h"
#include "App/GraphicsDevice.h"
#include "App/CommandQueue.h"
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
	static constexpr DWORD SCONST_WINDOW_STYLE = WS_OVERLAPPEDWINDOW;
	static constexpr UINT  SCONST_BACK_BUFFER_COUNT = 3u;
	static constexpr bool  SCONST_IS_VSYNC = true;

	struct HwndDeleter
	{
		void operator()(HWND hwnd) const noexcept
		{
			if (hwnd && ::IsWindow(hwnd))
				::DestroyWindow(hwnd);
		}
	};

	using UniqueHWND = std::unique_ptr<std::remove_pointer_t<HWND>, HwndDeleter>;
	using SwapChain = Microsoft::WRL::ComPtr<IDXGISwapChain4>;
	using BackBufferResource = std::array<Microsoft::WRL::ComPtr<ID3D12Resource>, SCONST_BACK_BUFFER_COUNT>;
	using BackBufferStatus = std::array<UINT64, SCONST_BACK_BUFFER_COUNT>;
	using ClearColor = std::array<FLOAT, 4ull>;

public:

	RenderWindow() = delete;
	RenderWindow(
		AppState& events,
		GraphicsDevice& device,
		RENDER_WINDOW_DESC desc
	);
	~RenderWindow();

	RenderWindow(const RenderWindow&) = delete;
	RenderWindow& operator=(const RenderWindow&) = delete;
	RenderWindow(RenderWindow&&) = delete;
	RenderWindow& operator=(RenderWindow&&) = delete;

	void begin(); // must be called before any execution or present

	CommandList get_command_list(); // get command lists to write into

	void submit_work(CommandList&& list); // write as many command list as desired to the command queue

	void present(); // call this to present to screen at the end of the frame

	void resize(UINT client_width, UINT client_height); // resize resolving method

	void toggle_fullscreen(bool fullscreen); // toggles window mode fullscreen on/off

	ID3D12Resource* get_buffer() const;
	constexpr D3D12_CPU_DESCRIPTOR_HANDLE get_buffer_desc() const noexcept { return mBufferViews.descriptor_at(mBufferIndex); }
	constexpr UINT    get_buffer_width()  const noexcept { return mWidth; }
	constexpr UINT    get_buffer_height() const noexcept { return mHeight; }

	void set_clear_color(float r, float g, float b, float a) { mClearColor = { r,g,b,a }; }

	constexpr const AppState& get_states() const noexcept { return mState; } // idk, temporary for demo to work simpler

protected:

	bool create_hwnd(const RENDER_WINDOW_DESC& desc);

	static LRESULT CALLBACK wnd_proc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
	{
		RenderWindow* self = nullptr;
		if (uMsg == WM_NCCREATE) {
			CREATESTRUCT* pCreate = (CREATESTRUCT*)lParam;
			self = (RenderWindow*)pCreate->lpCreateParams;
			SetWindowLongPtrW(hwnd, GWLP_USERDATA, (LONG_PTR)self);
		}
		else {
			self = (RenderWindow*)GetWindowLongPtrW(hwnd, GWLP_USERDATA);
		}

		if (self)
			return self->on_message(hwnd, uMsg, wParam, lParam);

		return DefWindowProcW(hwnd, uMsg, wParam, lParam);
	}

	LRESULT on_message(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);


	void update_back_buffers();
	void update_current_index();
	constexpr UINT64 current_buffer_signal() const noexcept { return mBufferSignals[mBufferIndex]; }
	constexpr void set_current_buffer_signal(const UINT64 value) noexcept{ mBufferSignals[mBufferIndex] = value; }

private:

	// window & swapchain
	UniqueHWND   mHwnd;
	SwapChain    mSwapChain;
	RECT         mSavedWindowRect;
	bool         mIsFullscreen;
	bool         mIsTearingSupported;
	bool         mInitialised;
	ClearColor   mClearColor;


	// buffer shit
	DescriptorHeap      mBufferViews;
	BackBufferResource  mBuffers;
	BackBufferStatus    mBufferSignals;
	UINT                mBufferIndex;
	UINT                mWidth;
	UINT                mHeight;

	// borrowed stuff from device
	ViewPtr<ID3D12Device4> mDevice;
	ViewPtr<CommandQueue> mQueue;
	AppState& mState;
};