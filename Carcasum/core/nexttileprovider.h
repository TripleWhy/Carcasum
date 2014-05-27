#ifndef NEXTTILEPROVIDER_H
#define NEXTTILEPROVIDER_H

#include "static.h"
#include "random.h"

class Game;
struct MoveHistoryEntry;

class NextTileProvider
{
public:
	virtual int nextTile(Game const * game) = 0;
};

class RandomNextTileProvider : public NextTileProvider
{
private:
	DefaultRandom r;
public:
	virtual int nextTile(Game const * game);
};

class HistoryProvider : public NextTileProvider
{
private:
	NextTileProvider * ntp;
	std::vector<MoveHistoryEntry> history;
	size_t offset;

public:
	HistoryProvider(NextTileProvider * ntp, std::vector<MoveHistoryEntry> const & history, size_t offset);
	virtual int nextTile(Game const * game);
};

#endif // NEXTTILEPROVIDER_H
