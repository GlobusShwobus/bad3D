#pragma once

#include "IGame.h"
#include "TypeRect.h"
#include "Utils.h"
#include <string>

class Demo1 :public IGame
{
public:


	WINDOW_EX_DESC make_create_window_desc() override
	{
		const DWORD window_style = WS_OVERLAPPEDWINDOW;
		LRect size = get_adjusted_window_rect(1280, 720, window_style);
		center_rect_in_display(size);


		WINDOW_EX_DESC desc = {};
		desc.window_name = L"demo 1";
		desc.window_style = window_style;
		desc.x = size.x;
		desc.y = size.y;
		desc.w = size.w;
		desc.h = size.h;

		return desc;
	}

	void load_content()override {};
	void unload_content()override {};

	void on_update(float dt)override {};
};