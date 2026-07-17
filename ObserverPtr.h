#pragma once

#include <cassert>
#include <cstddef>

template <typename T>
class ObserverPtr
{
public:
	using element_type = T;

	constexpr ObserverPtr() noexcept = default;
	constexpr ObserverPtr(std::nullptr_t) noexcept {}
	constexpr explicit ObserverPtr(T* ptr) noexcept
		: ptr_(ptr)
	{
	}

	constexpr ObserverPtr(const ObserverPtr&) noexcept = default;
	constexpr ObserverPtr& operator=(const ObserverPtr&) noexcept = default;

	constexpr ObserverPtr& operator=(std::nullptr_t) noexcept
	{
		ptr_ = nullptr;
		return *this;
	}

	constexpr void observe_this(T* ptr = nullptr) noexcept
	{
		ptr_ = ptr;
	}

	constexpr T* stop_observing() noexcept
	{
		T* tmp = ptr_;
		ptr_ = nullptr;
		return tmp;
	}

	constexpr T* get() const noexcept
	{
		return ptr_;
	}

	constexpr explicit operator bool() const noexcept
	{
		return ptr_ != nullptr;
	}

	constexpr T& operator*() const
	{
		assert(ptr_);
		return *ptr_;
	}

	constexpr T* operator->() const noexcept
	{
		return ptr_;
	}

private:
	T* ptr_ = nullptr;
};