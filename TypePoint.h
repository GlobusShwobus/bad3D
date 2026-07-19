#pragma once

template<typename T>
struct TypePoint
{
	T x;
	T y;
};

using FPoint = TypePoint<float>;
using IPoint = TypePoint<int>;
using LPoint = TypePoint<long>;