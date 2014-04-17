#ifndef UTIL_H
#define UTIL_H

#include "static.h"
#include "tile.h"

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wconversion"
#include <QPoint>
#include <QThread>
#include <QCoreApplication>
#pragma GCC diagnostic pop

#include <map>
#include <thread>
#include <chrono>

typedef QVarLengthArray<int, MAX_PLAYERS> RewardType;

namespace Util
{
	inline bool isGUIThread()
	{
		return QThread::currentThread() == QCoreApplication::instance()->thread();
	}

	inline bool isNodeFree(Node const * n)
	{
//		return true;
		return !n->isOccupied();
	}

	inline void sleep(int millis)
	{
		std::chrono::milliseconds dura( millis );
		std::this_thread::sleep_for( dura );
	}

	inline Tile::TileSet getSet(TileTypeType tileType)
	{
		Q_UNUSED(tileType);
		return Tile::BaseGame;
	}

	inline TileTypeType toGlobalType(Tile::TileSet set, TileTypeType setType)
	{
		switch (set)
		{
			case Tile::BaseGame:
				return setType + 0;
		}
		return -1;
	}

	inline TileTypeType toLocalType(TileTypeType globalType)
	{
		switch (getSet(globalType))
		{
			case Tile::BaseGame:
				return globalType - 0;
		}
		return -1;
	}

	inline QString getDisplayName(Tile::TileSet s)
	{
		switch(s)
		{
			case Tile::BaseGame:
				return "Base Game";
		}
		return "";
	}

	inline int utilitySimple(int const * scores, int const playerCount, int const myIndex)
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

	inline int utilityComplexOld(int const * scores, int const playerCount, int const myIndex)
	{
		std::multimap<int, int> map;
		for (int i = 0; i < playerCount; ++i)
			map.insert(std::pair<int, int>(scores[i], i));

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

	inline int utilityComplex(int const * scores, int const playerCount, int const myIndex)
	{
		std::multimap<int, int> map;
		for (int i = 0; i < playerCount; ++i)
			map.insert(std::pair<int, int>(scores[i], i));

		int u = 0;
		int m = 0;
		int lastScore = -1;
		int index = 1;
		// last player first
		for (auto const & e : map)
		{
			if (e.first != lastScore)
			{
				int count = (int)map.count(e.first);
				m = index * index;
				for (int i = 1; i < count; ++i)
					m += (index+i)*(index+i);
				m /= count;
			}

			if (e.second == myIndex)
				u += m * e.first;
			else
				u -= m * e.first;
			lastScore = e.first;
			++index;
		}

		return u;
	}

	inline qreal utilityComplexF(int const * scores, int const playerCount, int const myIndex)
	{
		std::multimap<int, int> map;
		for (int i = 0; i < playerCount; ++i)
			map.insert(std::pair<int, int>(scores[i], i));

		qreal u = 0;
		qreal m = 0;
		int lastScore = -1;
		int index = 1;
		// last player first
		for (auto const & e : map)
		{
			if (e.first != lastScore)
			{
				int count = (int)map.count(e.first);
				m = index * index;
				for (int i = 1; i < count; ++i)
					m += (index+i)*(index+i);
				m /= count;
			}

			if (e.second == myIndex)
				u += m * e.first;
			else
				u -= m * e.first;
			lastScore = e.first;
			++index;
		}

		return u;
	}

	inline int utilityUpperBound(int const playerCount, int const upperScoreBound)
	{
//		return playerCount * upperScoreBound;
		int scores[MAX_PLAYERS] = { upperScoreBound };
		return utilityComplex(scores, playerCount, 0);
	}

	inline int utilityLowerBound(int const playerCount, int const upperScoreBound)
	{
//		return -((upperScoreBound * (playerCount*playerCount + playerCount - 2)) / 2);	// == sum(upperScoreBound*i), for i from 2 to playerCount.
		int scores[MAX_PLAYERS] = { };
		for (int i = 1; i < playerCount; ++i)
			scores[i] = upperScoreBound;
		return utilityComplex(scores, playerCount, 0);
	}

	inline qreal utilityUpperBoundF(int const playerCount, int const upperScoreBound)
	{
		int scores[MAX_PLAYERS] = { 0 };
		scores[0] = upperScoreBound;
		return utilityComplexF(scores, playerCount, 0);
	}

	inline qreal utilityLowerBoundF(int const playerCount, int const upperScoreBound)
	{
		int scores[MAX_PLAYERS] = { upperScoreBound };
		scores[0] = 0;
		return utilityComplexF(scores, playerCount, 0);
	}

	inline qreal utilityComplexNormalized(int const * scores, int const playerCount, int const myIndex, int const upperScoreBound)
	{
		int const u = utilityComplex(scores, playerCount, myIndex);

		int const uBound = utilityUpperBound(playerCount, upperScoreBound);
		int const lBound = utilityLowerBound(playerCount, upperScoreBound);
		int const range = uBound - lBound;
		qreal const A = (range * playerCount) / 300.0;
		qreal const B = (6 * playerCount) / qreal(range);

		return tanh(((u+A) * B) + 1) / 2;
	}

	inline qreal * newUtilityComplexNormalizationTable(int const playerCount, int const upperScoreBound, int & size, int & offset)
	{

		int const uBound = utilityUpperBound(playerCount, upperScoreBound);
		int const lBound = utilityLowerBound(playerCount, upperScoreBound);
		int const range = uBound - lBound;
		qreal const A = (range * playerCount) / 300.0;
		qreal const B = (6 * playerCount) / qreal(range);

		size = range + 1;
		offset = -lBound;
		qreal * table = new qreal[size];
		for (int i = lBound; i <= uBound; ++i)
			table[i - lBound] = tanh(((i+A) * B) + 1) / 2;
		return table;
	}

	inline RewardType utilitySimpleMulti(int const * scores, int const playerCount)
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

	void syncGamesFast(Game const & from, Game & to);
	void syncGames(Game const & from, Game & to);

	template<typename T>
	class OffsetArray
	{
	private:
		T * arr;
		int _size;
		int _offset;

	public:
		constexpr OffsetArray(T * arr, int size, int offset) : arr(arr + offset), _size(size), _offset(offset) {}
		constexpr OffsetArray() : arr(0), _size(0), _offset(0) {}
		OffsetArray & operator=(const OffsetArray<T> &) = default;

	#if OFFSET_ARRAY_ENABLE_CHECKS
		const T & operator[](int const & index) const
		{
			rangeCheck(index);
			return arr[index];
		}
	#else
		constexpr const T & operator[](int const & index) const
		{
			return arr[index];
		}
	#endif
		constexpr explicit operator bool() const
		{
			return arr;
		}
		constexpr int size() const
		{
			return _size;
		}

	#if OFFSET_ARRAY_ENABLE_CHECKS
		void rangeCheck(int const & index) const
		{
			int chkIndex = index + _offset;
	 #if defined(QT_NO_DEBUG) && !defined(QT_FORCE_ASSERTS)
			if (chkIndex < 0 || chkIndex >= _size)
			{
				qFatal(QString("Index out of bounds: %1").arg(index).toStdString().c_str());
				exit(1);
			}
	 #else
			Q_ASSERT(chkIndex >= 0);
			Q_ASSERT(chkIndex < _size);
	 #endif
		}
	#endif
	};

	class Math
	{
	private:
		static OffsetArray<qreal> lnTable;

	public:
		Math()
		{
			if (!lnTable)
			{
				qreal * table = new qreal[LN_TABLE_SIZE];
				for (uint i = 0; i < LN_TABLE_SIZE; ++i)
					table[i] = ::log(i);
				lnTable = OffsetArray<qreal>(table, LN_TABLE_SIZE, 0);
			}
		}

		inline qreal sqrt(qreal r) const
		{
			return ::sqrt(r);	// This seems to be already the fastest possible implementation. Whatever it does.
		}

		inline qreal ln(uint r) const
		{
			if (r >= LN_TABLE_SIZE)
				return ::log(r);
			return lnTable[r];
		}
	};
	Math const mathInstance;


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
}

#endif // UTIL_H
