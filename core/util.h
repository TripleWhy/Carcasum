#ifndef UTIL_H
#define UTIL_H

#include "static.h"
#include "tile.h"

#include <QPoint>
#include <QThread>
#include <QCoreApplication>

#include <thread>
#include <chrono>

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
