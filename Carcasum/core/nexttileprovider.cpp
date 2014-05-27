#include "nexttileprovider.h"
#include "game.h"

int RandomNextTileProvider::nextTile(const Game * game)
{
	return r.nextInt(game->getTileCount());
}


HistoryProvider::HistoryProvider(NextTileProvider * ntp, const std::vector<MoveHistoryEntry> & history, size_t offset)
    : ntp(ntp),
      history(history),
      offset(offset)
{
}

int HistoryProvider::nextTile(const Game * game)
{
	if (offset >= history.size())
		return ntp->nextTile(game);
	return history[offset++].tileIndex;
}
