#pragma once

template<typename T>
struct TypeRect
{
	T x;
	T y;
	T w;
	T h;
};

using FRect = TypeRect<float>;
using UIRect = TypeRect<unsigned int>;
using LRect = TypeRect<long>;