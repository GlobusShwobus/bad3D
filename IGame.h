#pragma once

#include "WIN32_CORE.h"
#include "GPU_CORE.h"

class IGame
{
public:
	virtual ~IGame() = default;

	// content loading / unloading
	virtual void setup_IGame(
	ObserverPtr<ID3D12Device4> device,
	ObserverPtr<DX>
	) = 0;
	virtual void unload_content() = 0;

	// on game specific logic update and rendering update
	virtual void on_update(float dt) = 0;
	virtual void on_render( ObserverPtr<ID3D12GraphicsCommandList2> cl) = 0;

	// key and mouse events
	virtual void on_key_event(UINT uMsg, WPARAM wParam, LPARAM lParam) {}
	virtual void on_mouse_event(UINT uMsg, WPARAM wParam, LPARAM lParam) {}
	
	// when application infra resizes, the game might also need to know about this
	virtual void on_window_resize(int width, int height) {}

protected:

	virtual void set_transition_barrier(
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
};
