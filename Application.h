#pragma once

#include "WIN32_CORE.h"
#include "GPU_CORE.h"

#include <memory>
#include <string>

#include "IWindowEventListener.h"
#include "DX12CommandQueue.h"
#include "DX12RenderWindow.h"
#include "IGame.h"

class Application final: public IWindowEventListener
{
	// windows advanced rasterization protocol
	static constexpr bool using_WARP_adapter = false;

public:

	Application() = default;

	// doesnt make sense for these 
	Application(const Application&) = delete;
	Application& operator=(const Application&) = delete;
	Application(Application&&) = delete;
	Application& operator=(Application&&) = delete;

	virtual ~Application();

	void initialise_dx12();
	void initialise_render_window(const std::wstring& title, UINT x, UINT y, UINT w, UINT h, DWORD window_style = WS_OVERLAPPEDWINDOW);
	

	ObserverPtr<ID3D12Device4> get_device() const noexcept { return mDevice.Get(); }
	ObserverPtr<DX12RenderWindow> get_window() const noexcept { return mDXWindow.get(); }
	ObserverPtr<DX12CommandQueue> get_command_queue(D3D12_COMMAND_LIST_TYPE type) const noexcept
	{
		ObserverPtr<DX12CommandQueue> p = nullptr;

		if (type == D3D12_COMMAND_LIST_TYPE_DIRECT)
			p = mDirectCommandQueue.get();
		else if (type == D3D12_COMMAND_LIST_TYPE_COMPUTE)
			p = mComputeCommandQueue.get();
		else if (type == D3D12_COMMAND_LIST_TYPE_COPY)
			p = mCopyCommandQueue.get();

		return p;
	}


	void set_game(ObserverPtr<IGame> game);
	void run();

protected:

	LRESULT on_message(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) override;

private:

	DXGIFactory4                        mFactory;
	D3D12Device4                        mDevice;

	std::unique_ptr<DX12CommandQueue>   mDirectCommandQueue;
	std::unique_ptr<DX12CommandQueue>   mComputeCommandQueue;
	std::unique_ptr<DX12CommandQueue>   mCopyCommandQueue;
	std::unique_ptr<DX12RenderWindow>   mDXWindow;

	ObserverPtr<IGame> mGame;

	bool mIsDX12Initalised = false;
};