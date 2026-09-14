#pragma once
#include "badWin32.h"

class IGame
{
public:
	virtual ~IGame() = default;

	// content loading / unloading
	virtual void load_content() = 0;
	virtual void unload_content() = 0;

	// on game specific logic update and rendering update
	virtual void on_update(  ) = 0;
	virtual void on_render(  ) = 0;
	virtual void on_resize( int w, int h ) = 0;
};
