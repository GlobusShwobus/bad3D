#pragma once

#include "WIN32_CORE.h"
#include "GPU_CORE.h"

#include <memory>

#include "IWindowEventListener.h"
#include "DX12CommandQueue.h"
#include "DX12RenderWindow.h"
#include "RenderFrameContext.h"
#include "Stopwatch.h"
#include "IGame.h"

class Application final: public IWindowEventListener
{
	// the number of back buffers for the swap chain aka surfaces aka drawable frames
	static constexpr unsigned int number_of_back_buffers = 3;

	// windows advanced rasterization protocol
	static constexpr bool using_WARP_adapter = false;

public:

	explicit Application(const WINDOW_CREATE_DESC& window_desc);

	// doesnt make sense for these 
	Application(const Application&) = delete;
	Application& operator=(const Application&) = delete;
	Application(Application&&) = delete;
	Application& operator=(Application&&) = delete;

	virtual ~Application();

	void bind_game(std::unique_ptr<IGame> game);

	void run();

protected:

	void reset_rendering();

	void finish_rendering();

	LRESULT on_message(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) override;

	void reset_signal_tracker();

	void on_user_render();

private:

	DXGIFactory4                        mFactory;
	D3D12Device4                        mDevice;

	std::unique_ptr<DX12CommandQueue>   mCommandQueue;
	std::unique_ptr<DX12RenderWindow>   mDXWindow;

	ObserverPtr<ID3D12GraphicsCommandList2>		    mCommandList;
	UINT64                                          mSignalTracker[number_of_back_buffers];

	std::unique_ptr<IGame> mGame = nullptr;
	std::unique_ptr<Stopwatch> mTimer = nullptr;

	bool mIsRuntimeSet = false;
	bool mIsGameBound = false;
};
