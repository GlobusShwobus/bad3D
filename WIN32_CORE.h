#pragma once

// trim rare windows API
#define WIN32_LEAN_AND_MEAN

// prevent windows defining min and max macros
#define NOMINMAX

// supress warnings that come from windows header itself and include windows
#pragma warning(push, 0)
#include <windows.h>
#pragma warning(pop)

static HINSTANCE g_hModule = nullptr;


struct WINDOW_CREATE_DESC
{
    PCWSTR window_name;
    DWORD  window_style;
    UINT x;
    UINT y;
    UINT w;
    UINT h;
    bool set_fullscreen;
    bool set_vsync;
};