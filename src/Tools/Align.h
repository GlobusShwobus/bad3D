#pragma once

/***************************************************************************
* These functions were taken from the MiniEngine.
* Source code available here:
* https://github.com/Microsoft/DirectX-Graphics-Samples/blob/master/MiniEngine/Core/Math/Common.h
* Retrieved: January 13, 2016
**************************************************************************/

template <typename T>
constexpr T align_up(T value, size_t alignment) noexcept
{
    alignment -= 1;
    return (T)(((size_t)value + alignment) & ~alignment);
}

template <typename T>
constexpr T align_down(T value, size_t alignment) noexcept
{
    alignment -= 1;
    return (T)((size_t)value & ~alignment);
}

template <typename T>
constexpr bool is_aligned(T value, size_t alignment) noexcept
{
    return 0 == ((size_t)value & (alignment - 1));
}

template <typename T>
constexpr T divide_by_multiple(T value, size_t alignment) noexcept
{
    return (T)((value + alignment - 1) / alignment);
}

/**
* Round up to the next highest power of 2.
* @source: http://graphics.stanford.edu/~seander/bithacks.html#RoundUpPowerOf2
* @retrieved: January 16, 2016
*/
constexpr unsigned int next_pow2_32bit(unsigned int v) noexcept
{
    v--;
    v |= v >> 1;
    v |= v >> 2;
    v |= v >> 4;
    v |= v >> 8;
    v |= v >> 16;
    v++;

    return v;
}

/**
* Round up to the next highest power of 2.
* @source: http://graphics.stanford.edu/~seander/bithacks.html#RoundUpPowerOf2
* @retrieved: January 16, 2016
*/
constexpr unsigned long long next_pow2_64bit(unsigned long long v) noexcept
{
    v--;
    v |= v >> 1;
    v |= v >> 2;
    v |= v >> 4;
    v |= v >> 8;
    v |= v >> 16;
    v |= v >> 32;
    v++;

    return v;
}