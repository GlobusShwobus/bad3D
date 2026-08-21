#pragma once

#include "badWin32.h"
#include "badDirectX.h"
#include <wrl/client.h>

#include "ViewPtr.h"

class RenderWindow final
{
	static constexpr UINT back_buffer_count = 3;

	struct DescHeap
	{
		Microsoft::WRL::ComPtr<ID3D12DescriptorHeap>   mDescriptorHeap = nullptr;
		UINT                                           mDescriptorSize = 0;

		void init(ViewPtr<ID3D12Device4> device, D3D12_DESCRIPTOR_HEAP_TYPE type, UINT count);
		D3D12_CPU_DESCRIPTOR_HANDLE get_descriptor_handle_for(SIZE_T index) const noexcept;
	};

	struct ScreenToggle
	{
		RECT   window_rect{ 0,0,0,0 };
		UINT   window_style = 0;
		bool   is_fullscreen = false;

		void toggle_window_to( HWND hwnd, bool mode );
	};

public:

	RenderWindow(
		ViewPtr<HWND__> hwnd,
		ViewPtr<IDXGIFactory4> factory,
		ViewPtr<ID3D12Device4> device,
		ViewPtr<ID3D12CommandQueue> command_queue,
		DWORD window_style = WS_OVERLAPPEDWINDOW
	);

	RenderWindow(const RenderWindow&) = delete;
	RenderWindow& operator=(const RenderWindow&) = delete;
	RenderWindow(RenderWindow&&) = delete;
	RenderWindow& operator=(RenderWindow&&) = delete;

	virtual ~RenderWindow() = default;

	void present_to_display();
	void resize( UINT client_width, UINT client_height);
	void toggle_fullscreen(bool fullscreen);

	ViewPtr<ID3D12Resource>     get_buffer() const;
	D3D12_CPU_DESCRIPTOR_HANDLE get_buffer_desc() const;

	constexpr UINT              get_buffer_index()  const noexcept  { return mCurrentBBIndex;   }
	constexpr UINT              get_buffer_count()  const noexcept  { return back_buffer_count; }
	constexpr UINT              get_buffer_width()  const noexcept  { return mBBWidth;          }
	constexpr UINT              get_buffer_height() const noexcept  { return mBBHeight;         }

	RECT                        get_client_rect() const;

protected:

	ViewPtr<ID3D12Resource>  get_buffer_at(UINT index) const;

	void reset_description_info() const;

private:

	// big core stuff
	ViewPtr<ID3D12Device4>                  mDevice    = nullptr;
	ViewPtr<HWND__>                         mHwnd      = nullptr;
	Microsoft::WRL::ComPtr<IDXGISwapChain4> mSwapChain = nullptr;

	// buffer info
	UINT mCurrentBBIndex = 0;
	UINT mBBWidth = 0;
	UINT mBBHeight = 0;

	// stuffz
	DescHeap     mDescHeap;
	ScreenToggle mScreenToggle;
	
	// settings
	bool mIsVSync            = false;
	bool mIsTearingSupported = false;
};