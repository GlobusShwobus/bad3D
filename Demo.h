#pragma once

#include "IGame.h"
#include "Utils.h"
#include <string>
#include "Keyboard.h"
#include "Mouse.h"

class Demo :public IGame
{
public:

	~Demo()override
	{
		unload_content();
	}
	void load_content()override {};
	void unload_content()override {};

	void on_update(float dt)override
	{
		mouse_resolve();
		mouse.update_mouse_buttons(dt);
	}

	void on_render(const RenderFrameContext& frame_context) override
	{

		frame_context.command_list->ClearRenderTargetView(frame_context.resource_desc, color, 0, nullptr);
	}

	void on_key_event(UINT uMsg, WPARAM wParam, LPARAM lParam) override
	{
		kb.resolve_message(uMsg, wParam, lParam);
	}

	void on_mouse_event(UINT uMsg, WPARAM wParam, LPARAM lParam) 
	{
		mouse.resolve_message(uMsg, wParam, lParam);
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

private:
	FLOAT color[4] = { 1.0f, 0.0f, 0.0f, 1.0f };
	Keyboard kb;
	Mouse mouse;
};