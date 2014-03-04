#ifndef UTIL_H
#define UTIL_H

#include "tile.h"

#include <QPoint>
#include <QThread>
#include <QCoreApplication>

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
	inline static bool isGUIThread()
	{
		return QThread::currentThread() == QCoreApplication::instance()->thread();
	}

	inline static bool isNodeFree(Node const * n)
	{
		//TODO
		return true;
	}
};

#endif // UTIL_H
