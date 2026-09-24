#pragma once

#include <concepts>
#include <chrono>

// This class gives only delta times from the last time a method was called.

class Stopwatch final
{
	using Valuesec = std::chrono::duration<double>;
	using Millisec = std::chrono::milliseconds;
	using Microsec = std::chrono::microseconds;
	using Nanosec  = std::chrono::nanoseconds;

public:
	explicit Stopwatch()noexcept
		:time_point(std::chrono::steady_clock::now())
	{
	}

	inline double delta() noexcept
	{
		return std::chrono::duration_cast<Valuesec>(elapsed()).count();
	}

	inline std::size_t millisec() noexcept
	{
		return std::chrono::duration_cast<Millisec>(elapsed()).count();
	}

	inline std::size_t microsec() noexcept
	{
		return std::chrono::duration_cast<Microsec>(elapsed()).count();
	}

	inline std::size_t nanosec() noexcept
	{
		return std::chrono::duration_cast<Nanosec>(elapsed()).count();
	}

	inline void reset()noexcept
	{
		time_point = std::chrono::steady_clock::now();
	}

protected:

	inline std::chrono::steady_clock::duration elapsed() noexcept
	{
		const auto old = time_point;
		time_point = std::chrono::steady_clock::now();
		return time_point - old;
	}

private:
	std::chrono::steady_clock::time_point time_point;
};

/**
* Times any function (is not actually ideal to measure things).
*
* \param func invokable method: function, functor, lambda
* \param args arguments to be passed to the func using perfect forwarding
* \returns double time it took to execute
* \returns if func return type is void, returns time
* \returns if func return type is T, returns std::pair<time, T>
*/
template <typename Func, typename... Args>
auto time_my_func(Func&& func, Args&&... args)
	requires std::invocable<Func, Args...>
{
	Stopwatch timer;

	if constexpr (std::is_void_v<std::invoke_result_t<Func, Args...>>)
	{
		std::invoke(std::forward<Func>(func), std::forward<Args>(args)...);
		return timer.delta();
	}
	else
	{
		auto result = std::invoke(std::forward<Func>(func), std::forward<Args>(args)...);

		return std::pair{ timer.delta(), std::move(result) };
	}
}