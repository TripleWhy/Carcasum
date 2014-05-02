#include "nexttileprovider.h"
#include "game.h"

int RandomNextTileProvider::nextTile(const Game * game)
{
	return r.nextInt(game->getTileCount());
}
