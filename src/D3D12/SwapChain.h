#pragma once

#include <d3d12.h>
#include <dxgi1_6.h>
#include <wrl/client.h>

#include "App/badWin32.h"
#include "Tools/ViewPtr.h"
#include "D3D12/DescriptorHeap.h"
#include "D3D12/CommandQueue.h"

class SwapChain final
{
	static constexpr UINT back_buffer_count = 3;

	struct ScreenToggle
	{
		RECT   window_rect{ 0,0,0,0 };
		UINT   window_style = 0;
		bool   is_fullscreen = false;

		void toggle_window_to( HWND hwnd, bool mode );
	};

public:

	SwapChain(
		ViewPtr<ID3D12Device4> device,
		ViewPtr<HWND__> hwnd,
		ID3D12CommandQueue* command_queue,
		IDXGIFactory4* factory,
		DWORD window_style = WS_OVERLAPPEDWINDOW
	);

	SwapChain(const SwapChain&) = delete;
	SwapChain& operator=(const SwapChain&) = delete;
	SwapChain(SwapChain&&) = delete;
	SwapChain& operator=(SwapChain&&) = delete;

	UINT64 present_to_display(UINT64 signal);
	void resize(CommandQueue& queue, UINT client_width, UINT client_height);
	void toggle_fullscreen(bool fullscreen);

	ID3D12Resource*             get_buffer() const;
	D3D12_CPU_DESCRIPTOR_HANDLE get_buffer_desc() const;

	constexpr UINT              get_buffer_index()  const noexcept  { return mCurrentBufferIndex;   }
	constexpr UINT              get_buffer_count()  const noexcept  { return back_buffer_count;     }
	constexpr UINT              get_buffer_width()  const noexcept  { return mBufferWidth;          }
	constexpr UINT              get_buffer_height() const noexcept  { return mBufferHeight;         }
	constexpr UINT64            get_buffer_signal() const noexcept  { return mBackBufferCompletionTrackers[mCurrentBufferIndex]; }

protected:

	void update_back_buffers();

private:

	// big core stuff
	ViewPtr<ID3D12Device4>                  mDevice    = nullptr;
	ViewPtr<HWND__>                         mHwnd      = nullptr;
	Microsoft::WRL::ComPtr<IDXGISwapChain4> mSwapChain = nullptr;
	Microsoft::WRL::ComPtr<ID3D12Resource>  mBackBuffers[back_buffer_count];
	UINT64                                  mBackBufferCompletionTrackers[back_buffer_count];

	// buffer info
	UINT mCurrentBufferIndex = 0;
	UINT mBufferWidth = 0;
	UINT mBufferHeight = 0;

	// stuffz
	DescriptorHeap     mDescHeap;
	ScreenToggle       mScreenToggle;
	
	// settings
	bool mIsVSync            = false;
	bool mIsTearingSupported = false;
};