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
};

#endif // UTIL_H
