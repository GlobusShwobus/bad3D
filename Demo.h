#pragma once

#include "IGame.h"
#include "Utils.h"
#include <string>
#include "Keyboard.h"
#include "Mouse.h"
#include "Stopwatch.h"


#include <DirectXMath.h>
#include <vector>

class Demo :public IGame
{
public:
	Demo()
	{
		// check of directX math library support
		if (!DirectX::XMVerifyCPUSupport())
		{
			throw std::runtime_error("memes");
		}
	}
	~Demo()override
	{
		unload_content();
	}
	void load_content(Application& app)override
	{
		mDevice = app.get_device();
		mCommandQueue = app.get_command_queue(D3D12_COMMAND_LIST_TYPE_DIRECT);
		mWindow = app.get_window();

		mSignalTracker.resize(mWindow->get_buffer_count(), 0);
		mTimer.reset();
	}
	void unload_content() override {};

	void on_update()override
	{
		const float dt = mTimer.dt_float();

		mouse_resolve();
		kb_resolve();
		mouse.update_mouse_buttons(dt);
	}

	void on_render( ) override
	{
		ObserverPtr<ID3D12GraphicsCommandList2> command_list = mCommandQueue->get_command_list();
		ObserverPtr<ID3D12Resource> current_back_buffer = mWindow->get_buffer();
		D3D12_CPU_DESCRIPTOR_HANDLE buffer_desc = mWindow->get_buffer_desc();

		set_transition_barrier(command_list, current_back_buffer, D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_RENDER_TARGET);

		// any rendering logic goes here until another transition barrier
		command_list->ClearRenderTargetView(buffer_desc, color, 0, nullptr);

		set_transition_barrier(command_list, current_back_buffer, D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PRESENT);

		command_list->Close();

		const UINT64 current_index = mWindow->get_buffer_index();
		const UINT64 signal_val = mCommandQueue->execute();
		mSignalTracker[current_index] = signal_val;

		mWindow->present_to_display();

		const UINT64 some_new_buffer_index = mWindow->get_buffer_index();

		mCommandQueue->wait(mSignalTracker[some_new_buffer_index]);
	}

	void on_resize() override
	{
		UINT client_width, client_height;
		mWindow->get_client_size(client_width, client_height);

		const UINT buffer_width = mWindow->get_buffer_width();
		const UINT buffer_height = mWindow->get_buffer_height();

		if (buffer_width != client_width || buffer_height != client_height)
		{
			mCommandQueue->flush();

			const UINT current_val = mSignalTracker[mWindow->get_buffer_index()];

			for (auto& fence_val : mSignalTracker)
				fence_val = current_val;

			mWindow->resize(client_width,client_height);
		}
	}

	void on_key_event(UINT uMsg, WPARAM wParam, LPARAM lParam) override
	{
		kb.resolve_message(uMsg, wParam, lParam);
	}

	void on_mouse_event(UINT uMsg, WPARAM wParam, LPARAM lParam) 
	{
		mouse.resolve_message(uMsg, wParam, lParam);
	}

protected:

	void kb_resolve()
	{
		static bool fullscreen = false;
		static bool f11_previous = false;

		const bool f11_current = kb.get_keys()[VK_F11];

		if (f11_current && !f11_previous)
		{
			fullscreen = !fullscreen;
			mWindow->toggle_fullscreen(fullscreen);
		}

		f11_previous = f11_current;
	}

	void mouse_resolve()
	{
		if (mouse.get_button(MouseButtonType::Left).is_down())
		{
			color[0] = 1;
			color[1] = 0;
		}
		else
		{
			color[0] = 0;
			color[1] = 1;
		}
	}

	void set_transition_barrier(
		ObserverPtr<ID3D12GraphicsCommandList2> command_list,
		ObserverPtr<ID3D12Resource> back_buffer,
		D3D12_RESOURCE_STATES before,
		D3D12_RESOURCE_STATES after
	)
	{
		D3D12_RESOURCE_BARRIER barrier = {};

		barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
		barrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
		barrier.Transition.pResource = back_buffer.get();
		barrier.Transition.StateBefore = before;
		barrier.Transition.StateAfter = after;
		barrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
		command_list->ResourceBarrier(1, &barrier);
	}

private:
	FLOAT color[4] = { 1.0f, 0.0f, 0.0f, 1.0f };
	Keyboard kb;
	Mouse mouse;

	ObserverPtr<ID3D12Device4> mDevice;
	ObserverPtr<DX12CommandQueue> mCommandQueue;
	ObserverPtr<DX12RenderWindow> mWindow;

	std::vector<UINT64> mSignalTracker;
	Stopwatch mTimer;
};