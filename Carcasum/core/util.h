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
#include <atomic>

namespace Util
{
	inline bool isGUIThread()
	{
		return QThread::currentThread() == QCoreApplication::instance()->thread();
	}

	inline void processEvents(QEventLoop::ProcessEventsFlags flags = QEventLoop::AllEvents)
	{
		QCoreApplication::processEvents(flags);
	}

	inline void processEventsIfGUIThread()
	{
		if (isGUIThread())
			QCoreApplication::processEvents();
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
		OffsetArray & operator=(const OffsetArray<T> &) = delete;

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

		void clear()
		{
			delete[] (arr - _offset);
			arr = 0;
			_size = 0;
			_offset = 0;
		}

		void setData(T * data, int size, int offset)
		{
			delete[] (arr - _offset);
			arr = data + offset;
			_size = size;
			_offset = offset;
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

	OffsetArray<qreal> getUtilityMap(const Game * game);

	class Math
	{
	public:
		static Math const instance;

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
				lnTable.setData(table, LN_TABLE_SIZE, 0);
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

	class InterruptableThread : public QThread
	{
	private:
		std::atomic<bool> interruptionRequested;

	public:
		InterruptableThread(QObject * parent = 0) : QThread(parent), interruptionRequested(false) {}

		void interrupt()
		{
			interruptionRequested = true;
		}

		bool isInterrupted()
		{
			return interruptionRequested;
		}

		void clearInterrupt()
		{
			interruptionRequested = false;
		}

		static InterruptableThread * currentInterruptableThread()
		{
			return dynamic_cast<InterruptableThread *>(QThread::currentThread());
		}
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
}

#endif // UTIL_H
