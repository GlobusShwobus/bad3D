#pragma once
#include <chrono>
#include <concepts>

/// <summary> Basic wrapper around std::chrono only doing time measurements. </summary>
class Stopwatch final
{
	using Valuesec = std::chrono::duration<double>;
	using Millisec = std::chrono::milliseconds;
	using Microsec = std::chrono::microseconds;
	using Nanosec = std::chrono::nanoseconds;

	/// <summary>
	/// Measures the difference between now and the last time this function was called.
	/// </summary>
	/// <returns> std::steady_clock::duration difference between now and then</returns>
	std::chrono::steady_clock::duration elapsed()noexcept
	{
		const auto old = mWatch;
		mWatch = std::chrono::steady_clock::now();
		return mWatch - old;
	}

public:
	/// <summary> Intializes time measurements. Internally calls std::chrono::steady_clock::now() </summary>
	explicit Stopwatch()noexcept
		:mWatch(std::chrono::steady_clock::now())
	{
	}

	/// <returns> Difference between now and then in double </returns>
	inline double dt_float() noexcept
	{
		return std::chrono::duration_cast<Valuesec>(elapsed()).count();
	}

	/// <returns> Difference between now and then in size_t milliseconds </returns>
	inline std::size_t dt_millisec() noexcept
	{
		return std::chrono::duration_cast<Millisec>(elapsed()).count();
	}

	/// <returns> Difference between now and then in size_t microseconds </returns>
	inline std::size_t dt_microsec() noexcept
	{
		return std::chrono::duration_cast<Microsec>(elapsed()).count();
	}

	/// <returns> Difference between now and then in size_t nanoseconds </returns>
	inline std::size_t dt_nanosec() noexcept
	{
		return std::chrono::duration_cast<Nanosec>(elapsed()).count();
	}

	/// <summary> Sets the stopwatch to std::chrono::steady_clock::now() </summary>
	inline void reset()noexcept
	{
		mWatch = std::chrono::steady_clock::now();
	}

private:
	std::chrono::steady_clock::time_point mWatch;
};

/// <summary>
/// Times any function (is not actually ideal to measure things).
/// </summary>
/// <param name="func"> func invokable method: function, functor, lambda </param>
/// <param name="...args"> args arguments to be passed to the func using perfect forwarding </param>
/// <returns> returns a double time it took to execute the method if func return is void</returns>

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
		return timer.dt_float();
	}
	else
	{
		auto result = std::invoke(std::forward<Func>(func), std::forward<Args>(args)...);

		return std::pair{ timer.dt_float(), std::move(result) };
	}
}