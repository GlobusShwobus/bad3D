#ifndef MEME_UTILS_H
#define MEME_UTILS_H

#define WIN_LEAN_AND_MEAN
#include <Windows.h>
#include <stdexcept>

inline void throw_if_failed(HRESULT hr)
{
	if (FAILED(hr))
		throw std::runtime_error{ "" };
}

#endif