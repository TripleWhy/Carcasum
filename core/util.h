#ifndef UTIL_H
#define UTIL_H

#include "static.h"
#include "tile.h"

#include <QPoint>
#include <QThread>
#include <QCoreApplication>

#include <thread>
#include <chrono>

typedef QVarLengthArray<int, MAX_PLAYERS> RewardType;

class Util
{
public:
	inline static bool isGUIThread()
	{
		return QThread::currentThread() == QCoreApplication::instance()->thread();
	}

	inline static bool isNodeFree(Node const * n)
	{
//		return true;
		return !n->isOccupied();
	}
	
	inline static void sleep(int millis)
	{
		std::chrono::milliseconds dura( millis );
		std::this_thread::sleep_for( dura );
	}

	inline static Tile::TileSet getSet(TileTypeType tileType)
	{
		Q_UNUSED(tileType);
		return Tile::BaseGame;
	}

	inline static TileTypeType toGlobalType(Tile::TileSet set, TileTypeType setType)
	{
		switch (set)
		{
			case Tile::BaseGame:
				return setType + 0;
		}
		return -1;
	}

	inline static TileTypeType toLocalType(TileTypeType globalType)
	{
		switch (getSet(globalType))
		{
			case Tile::BaseGame:
				return globalType - 0;
		}
		return -1;
	}

	inline static QString getDisplayName(Tile::TileSet s)
	{
		switch(s)
		{
			case Tile::BaseGame:
				return "Base Game";
		}
		return "";
	}

	inline static int utilitySimple(int const * scores, int const playerCount, int const myIndex)
	{
		int best = std::numeric_limits<int>::min();
		int winner = -1;
		for (int i = 0; i < playerCount; ++i)
		{
			const int score = scores[i];
			if (score > best)
			{
				best = score;
				winner = i;
			}
			else if (score == best)
			{
				winner = -1;
			}
		}
		if (winner == myIndex)
			return 1;
		else if (winner == -1 && best == scores[myIndex])
			return 0;
		else
			return -1;
	}

	inline static int utilityComplex(int const * scores, int const playerCount, int const myIndex)
	{
		std::map<int, int> map;
		for (int i = 0; i < playerCount; ++i)
			map[scores[i]] = i;

		int u = 0;
		int m = 0;
		int lastScore = -1;
		// last player first
		for (auto const & e : map)
		{
			if (e.first != lastScore)
				++m;
			if (e.second == myIndex)
				u += m * e.first;
			else
				u -= m * e.first;
			lastScore = e.first;
		}

		return u;
	}

	inline static RewardType utilitySimpleMulti(int const * scores, int const playerCount)
	{
		RewardType reward(playerCount);
		int max = std::numeric_limits<int>::min();
		int winner = -1;
		for (int i = 0; i < playerCount; ++i)
		{
			int s = scores [i];
			if (s > max)
			{
				max = s;
				winner = i;
			}
			else if (s == max)
			{
				winner = -1;
			}
		}

		for (int i = 0; i < playerCount; ++i)
		{
			if (i == winner)
				reward[i] = 1;
			else if (winner == -1 && scores[i] == max)
				reward[i] = 0;
			else
				reward[i] = -1;
		}
		return reward;
	}

	inline static qreal mysqrt(qreal r) { return sqrt(r); }

	template<typename T>
	inline static qreal ln(T r) { return log(r); }

	static void syncGamesFast(Game const & from, Game & to);
	static void syncGames(Game const & from, Game & to);
};

// From http://qt-project.org/forums/viewthread/23849/#110462
template <typename T, typename U>
class FlagIterator
{
public:
    FlagIterator(T& flags) : mFlags((unsigned)flags), mFlag(0) {}
    inline U value() { return static_cast<U>(mFlag); }
    inline bool hasNext() { return mFlags > mFlag; }
    void next() { if(mFlag == 0) mFlag = 1; else mFlag <<= 1;
                  while((mFlags & mFlag) == 0) mFlag <<= 1; mFlags &= ~mFlag; }
private:
    unsigned mFlags;
    unsigned mFlag;
};


#endif // UTIL_H
