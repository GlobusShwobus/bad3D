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
#include "D3D12/DescriptorHeap.h"
#include "Tools/ViewPtr.h"

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


class CommandQueue;

class RenderWindow final
{
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
	RenderWindow(RenderWindow&&) noexcept = delete;
	RenderWindow& operator=(RenderWindow&&) noexcept = delete;

	void begin(); // must be called before any execution or present

	void present(); // call this to present to screen at the end of the frame

	void resize(UINT client_width, UINT client_height); // resize resolving method

	void toggle_fullscreen(bool fullscreen); // toggles window mode fullscreen on/off

	ID3D12Resource* get_buffer() const noexcept {
		return mBuffers[mBufferIndex].Get(); 
	}

	D3D12_CPU_DESCRIPTOR_HANDLE get_buffer_desc() const noexcept { 
		return mBufferViews.handle(mBufferIndex);
	}

	UINT get_buffer_width() const noexcept { 
		return mWidth;
	}

	UINT get_buffer_height() const noexcept {
		return mHeight;
	}

	void set_clear_color(float r, float g, float b, float a) noexcept { 
		mClearColor = { r,g,b,a }; 
	}

protected:

	bool create_hwnd(const RENDER_WINDOW_DESC& desc) noexcept;

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

	void update_current_index() { 
		mBufferIndex = mSwapChain->GetCurrentBackBufferIndex(); 
	}

	UINT64 current_buffer_signal() const noexcept { 
		return mBufferSignals[mBufferIndex]; 
	}

	void set_current_buffer_signal(const UINT64 value) noexcept {
		mBufferSignals[mBufferIndex] = value; 
	}

private:

	static constexpr const DWORD sccWindowStyle = WS_OVERLAPPEDWINDOW;
	static constexpr const UINT  sccBufferCount = 3u;
	static constexpr const bool  sccVSYNC = true;

	struct HwndDeleter
	{
		void operator()(HWND hwnd) const noexcept {
			if (hwnd && ::IsWindow(hwnd))
				::DestroyWindow(hwnd);
		}
	};

	using UniqueHWND = std::unique_ptr<std::remove_pointer_t<HWND>, HwndDeleter>;
	using SwapChain = Microsoft::WRL::ComPtr<IDXGISwapChain4>;
	using BackBufferResource = std::array<Microsoft::WRL::ComPtr<ID3D12Resource>, sccBufferCount>;
	using BackBufferStatus = std::array<UINT64, sccBufferCount>;
	using ClearColor = std::array<FLOAT, 4ull>;

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