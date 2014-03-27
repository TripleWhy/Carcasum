#ifndef NEXTTILEPROVIDER_H
#define NEXTTILEPROVIDER_H

#include "static.h"
#include "random.h"

class Game;

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

#endif // NEXTTILEPROVIDER_H
