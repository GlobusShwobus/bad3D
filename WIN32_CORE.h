#pragma once

// trim rare windows API
#define WIN32_LEAN_AND_MEAN

// prevent windows defining min and max macros
#define NOMINMAX

// supress warnings that come from windows header itself and include windows
#pragma warning(push, 0)
#include <windows.h>
#pragma warning(pop)

