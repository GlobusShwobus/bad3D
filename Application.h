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
	void initialise_IGame(std::unique_ptr<IGame> game);

	void free_IGame();

	ObserverPtr<DX12RenderWindow> get_render_window() noexcept
	{
		return mDXWindow.get();
	}

	ObserverPtr<ID3D12Device4> get_device() noexcept
	{
		return mDevice.Get();
	}

	ObserverPtr<DX12CommandQueue> get_command_queue() noexcept
	{
		return mCommandQueue.get();
	}

	void run();

protected:

	LRESULT on_message(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) override;

private:

	DXGIFactory4                        mFactory;
	D3D12Device4                        mDevice;

	std::unique_ptr<DX12CommandQueue>   mCommandQueue;
	std::unique_ptr<DX12RenderWindow>   mDXWindow;

	std::unique_ptr<IGame> mGame = nullptr;

	bool mIsDX12Initalised = false;
};