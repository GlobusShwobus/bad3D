#pragma once

#include <cassert>
#include <cstddef>

template <typename T>
class ObserverPtr
{
public:
	constexpr ObserverPtr() noexcept = default;
	constexpr ObserverPtr(std::nullptr_t) noexcept {}
	constexpr ObserverPtr(T* ptr) noexcept // let it be implicit for now
		: mPtr(ptr)
	{
	}

	constexpr ObserverPtr(const ObserverPtr&) noexcept = default;
	constexpr ObserverPtr& operator=(const ObserverPtr&) noexcept = default;

	constexpr ObserverPtr& operator=(std::nullptr_t) noexcept
	{
		mPtr = nullptr;
		return *this;
	}

	constexpr void observe_this(T* ptr) noexcept
	{
		mPtr = ptr;
	}

	constexpr T* stop_observing() noexcept
	{
		T* tmp = mPtr;
		mPtr = nullptr;
		return tmp;
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
		return mPtr;
	}

private:
	T* mPtr = nullptr;
};