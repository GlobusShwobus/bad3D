#pragma once

#include <cassert>
#include <cstddef>

template <typename T>
class ViewPtr
{
public:
	constexpr ViewPtr() noexcept = default;
	constexpr ViewPtr(std::nullptr_t) noexcept {}
	constexpr explicit ViewPtr(T* ptr) noexcept
		: mPtr(ptr)
	{
	}

	constexpr ViewPtr(const ViewPtr&) noexcept = default;
	constexpr ViewPtr& operator=(const ViewPtr&) noexcept = default;

	constexpr ViewPtr& operator=(std::nullptr_t) noexcept
	{
		mPtr = nullptr;
		return *this;
	}

	constexpr void set(T* ptr) noexcept
	{
		mPtr = ptr; 
	}
	constexpr T* get() const noexcept
	{
		return mPtr; 
	}
	constexpr explicit operator bool() const noexcept
	{
		return mPtr != nullptr; 
	}
	constexpr T& operator*() const 
	{
		assert(mPtr); 
		return *mPtr;
	}
	constexpr T* operator->() const noexcept 
	{
		assert(mPtr);
		return mPtr;
	}

private:
	T* mPtr = nullptr;
};