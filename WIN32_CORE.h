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

struct WINDOW_REGISTER_DESC
{
	LPCWSTR   class_name;
	HINSTANCE hInstance;
	DWORD     class_style;
	HICON     hIcon;
	HICON     hIconSm;
	HCURSOR   hCursor;
	HBRUSH    hbrBackground;
	LPCWSTR   lpszMenuName;
	int       cbClsExtra;
	int       cbWndExtra;
};

struct WINDOW_CREATE_DESC
{
	PCWSTR window_name;
	DWORD  window_style;
	UINT x;
	UINT y;
	UINT w;
	UINT h;
};

/*

GUIDE:

classes: 
		name:
			starts with a capital letter
			each word capitalized
			no underscores / spaces

		members:
			begins with m for for member
			each word capitalized
			no spaces

		organisation:
			using / type def / friend declarations at the top
			any nested classes / structs also at the top
			any static / constexpr members after type declarations
			public / protected / private
			member variables at the bottom

structs:

		name:
			starts with a capital letter
			each word capitalized
			no underscores / spaces

		members:
			simple naive names; no special letters
			if a name consists of more than one word use underscore
			no upper cases
			when it comes to external API, try and follow their naming comv (mainly windows)

		organisation:
			using / type def declarations at the top
			members after typedefs
			fully public

functions:
	
		all lower letters except abbreviations
		c++ style return prefered over C style param out


*/