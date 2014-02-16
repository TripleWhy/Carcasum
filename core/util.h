#ifndef UTIL_H
#define UTIL_H

#include <QPoint>

#include <random>
#include <chrono>

class Random
{
private:
	std::default_random_engine generator;
	std::uniform_int_distribution<int> distribution;
	static int add;

public:
	Random()
		: generator(std::chrono::system_clock::now().time_since_epoch().count() + (++add))
	{
		distribution.reset();
	}

	inline int nextInt()
	{
		return distribution(generator);
	}

	inline int nextInt(int max)
	{
		return nextInt() % max;
	}

	inline int nextInt(int low, int high)
	{
		return nextInt() % (high - low) + low;
	}
};

class Util
{
public:

};

#endif // UTIL_H
