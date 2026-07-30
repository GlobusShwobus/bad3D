#pragma once

#include "WIN32_CORE.h"
#include "ObserverPtr.h"
#include "GPU_CORE.h"
#include "RenderFrameContext.h"

class IGame
{
public:

	virtual WINDOW_EX_DESC make_create_window_desc() = 0;

	// content loading / unloading
	virtual void load_content() = 0;
	virtual void unload_content() = 0;

	// on game specific logic update and rendering update
	virtual void on_update(float dt) = 0;
	virtual void on_render(const RenderFrameContext& frame_context) = 0;

	// key and mouse events
	virtual void on_key_event(UINT uMsg, WPARAM wParam, LPARAM lParam) {}
	virtual void on_mouse_event(UINT uMsg, WPARAM wParam, LPARAM lParam) {}
};
