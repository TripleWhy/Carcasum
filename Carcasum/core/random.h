#ifndef RANDOM_H
#define RANDOM_H

#include "static.h"

#include <random>
#include <chrono>

#include <QtGlobal>

class Random
{
public:
	virtual ~Random() {}
	virtual int next() = 0;

	int nextInt(int max)
	{
		return next() % max;
	}

	inline int nextInt(int low, int high)
	{
		return next() % (high - low) + low;
	}

};

class DefaultRandom : public Random
{
private:
	std::default_random_engine generator;
	std::uniform_int_distribution<int> distribution;
	static int add;

public:
	DefaultRandom()
#ifndef RANDOM_SEED
		: generator(std::chrono::system_clock::now().time_since_epoch().count() + (++add))
#else
	    : generator(RANDOM_SEED + (++add))
#endif
	{
		distribution.reset();
	}

	inline int next()
	{
		return distribution(generator);
	}
};

class RandomTable : public Random
{
private:
	static int const TABLE_SIZE = 1 << 22;
	static int const ADD_MULTIPLIER = 31;
	static int * table;
	static int tableUse;
	static int add;
#if RANDOM_TABLE_SHARED_OFFSET
	static
#endif
	long offset;

public:
	RandomTable()
	{
		++tableUse;
		if (table == 0)
		{
			table = new int[TABLE_SIZE];
			DefaultRandom r;
			for (int i = 0; i < TABLE_SIZE; ++i)
			{
				table[i] = r.next();
			}
		}
#ifndef RANDOM_SEED
		long const seed = std::chrono::system_clock::now().time_since_epoch().count();
#else
		long const seed = RANDOM_SEED;
#endif
		add = (add + 1) % TABLE_SIZE;
		offset = (seed + (table[add] * ADD_MULTIPLIER));
		offset = (offset % TABLE_SIZE + TABLE_SIZE) % TABLE_SIZE;
	}

	~RandomTable()
	{
		if (--tableUse <= 0)
		{
			int * t = table;
			table = 0;
			delete[] t;
		}
	}

	inline virtual int next()
	{
#if RANDOM_TABLE_SHARED_OFFSET
		int o = (offset + 1) % TABLE_SIZE;
		offset = o;
#else
		if (Q_UNLIKELY(++offset >= TABLE_SIZE))
			offset -= TABLE_SIZE;
#endif
		return table[offset];
	}
};

#endif // RANDOM_H
