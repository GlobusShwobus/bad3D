#pragma once

#include <assert.h>

#include <random>

// This class acts like a distribution factory class really just wrapping std::mt19937. Pure syntactic sugar.
// The main pro of this wrapper is begin able to do the following:
//
// 
// Random random;
// 
// auto random_int = random.get_int_distribution(0, 10);
// 
// int my_random_int = random_int(random);
//
// Basically it just skips the std:: stuff

class Random final
{
public:
	using result_type = std::mt19937::result_type;

	static constexpr result_type min() { return std::mt19937::min(); }
	static constexpr result_type max() { return std::mt19937::max(); }

	result_type operator()()
	{
		return mEngine();
	}

	Random()
		:mEngine(std::random_device{}())
	{
	}

	int get(int min, int max)
	{
		assert(min <= max);
		return std::uniform_int_distribution<int>(min, max)(mEngine);
	}

	float get(float min, float max)
	{
		assert(min <= max);
		return std::uniform_real_distribution<float>(min, max)(mEngine);
	}

	auto get_int_distribution(int min, int max)
	{
		assert(min <= max);
		return std::uniform_int_distribution<int>(min, max);
	}

	auto get_real_distribution(float min, float max)
	{
		assert(min <= max);
		return std::uniform_real_distribution<float>(min, max);
	}

	auto get_normal_distribution(float mean, float standard_deviation)
	{
		return std::normal_distribution<float>(mean, standard_deviation);
	}

private:

	std::mt19937 mEngine;
};
