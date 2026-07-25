#pragma once

#include "IWin32Window.h"
#include "TypeRect.h"
#include "Utils.h"
#include <string>

class IGame
{
public:

	virtual WINDOW_REGISTER_DESC make_register_desc()
	{
		WINDOW_REGISTER_DESC desc = {};
		desc.class_name = L"IGameRegisterWindow";
		desc.class_style = CS_HREDRAW | CS_VREDRAW;
		desc.hInstance = ::GetModuleHandleW(nullptr);
		return desc;
	}

	virtual WINDOW_CREATE_DESC make_create_window_desc()
	{
		const DWORD window_style = WS_OVERLAPPEDWINDOW;
		LRect size = get_adjusted_window_rect(1280, 720, window_style);
		center_rect_in_display(size);
		

		WINDOW_CREATE_DESC desc = {};
		desc.class_name = L"IGameRegisterWindow";
		desc.window_name = L"IGameWindow";
		desc.window_style = window_style;
		desc.x = size.x;
		desc.y = size.y;
		desc.w = size.w;
		desc.h = size.h;
		desc.hInstance = ::GetModuleHandleW(nullptr);
		desc.start_fullscreen = false;

		return desc;
	}
};

//	
//	IGame(const std::wstring& window_title, uint32_t width, uint32_t height);
//	virtual ~IGame();
//
//
//
//
//	// Initialize the DirectX Runtime.
//	virtual bool initialize();
//
//    // Load content required for the demo.
//    virtual bool load_content() = 0;
//
//    // Unload demo specific content that was loaded in LoadContent.
//    virtual void unload_content() = 0;
//
//    // Destroy any resource that are used by the game.
//    virtual void destroy() = 0;
//
//    // updates the game logic
//    virtual void on_update();
//
//    /**
//     *  Render stuff.
//     */
//    virtual void OnRender(RenderEventArgs& e);
//
//    /**
//     * Invoked by the registered window when a key is pressed
//     * while the window has focus.
//     */
//    virtual void OnKeyPressed(KeyEventArgs& e);
//
//    /**
//     * Invoked when a key on the keyboard is released.
//     */
//    virtual void OnKeyReleased(KeyEventArgs& e);
//
//    /**
//     * Invoked when the mouse is moved over the registered window.
//     */
//    virtual void OnMouseMoved(MouseMotionEventArgs& e);
//
//    /**
//     * Invoked when a mouse button is pressed over the registered window.
//     */
//    virtual void OnMouseButtonPressed(MouseButtonEventArgs& e);
//
//    /**
//     * Invoked when a mouse button is released over the registered window.
//     */
//    virtual void OnMouseButtonReleased(MouseButtonEventArgs& e);
//
//    /**
//     * Invoked when the mouse wheel is scrolled while the registered window has focus.
//     */
//    virtual void OnMouseWheel(MouseWheelEventArgs& e);
//
//    /**
//     * Invoked when the attached window is resized.
//     */
//    virtual void OnResize(ResizeEventArgs& e);
//
//    /**
//     * Invoked when the registered window instance is destroyed.
//     */
//    virtual void OnWindowDestroy();
//
//private:
//    std::wstring m_Name;
//    int m_Width;
//    int m_Height;
//    bool m_vSync;