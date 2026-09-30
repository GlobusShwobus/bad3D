#pragma once

#include "App/GraphicsDevice.h"
#include "App/RenderWindow.h"
#include "App/Events.h"

class IScene
{
public:

	IScene(const AppState& read_events)
		:mState(read_events)
	{
	}
	
	virtual ~IScene() = default;

	// content loading / unloading
	virtual void load_content(GraphicsDevice* device, RenderWindow* window) = 0;
	virtual void unload_content() = 0;

	// on game specific logic update and rendering update
	virtual void on_update(  ) = 0;
	virtual void on_render(  ) = 0;
	virtual void on_resize( int w, int h ) = 0;

protected:
	const AppState& mState;
};
