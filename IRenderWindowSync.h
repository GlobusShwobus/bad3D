#ifndef I_RENDER_WINDOW_SYNC_H
#define I_RENDER_WINDOW_SYNC_H
struct InterfaceRenderWindowSync
{
	virtual void on_resize(unsigned int width, unsigned int height) = 0;
	virtual ~InterfaceRenderWindowSync() = default;
};
#endif