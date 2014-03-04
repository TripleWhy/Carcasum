#ifndef RANDOMPLAYER_H
#define RANDOMPLAYER_H

#include "core/player.h"
#include "core/util.h"

class RandomPlayer : public Player
{
private:
	Random r;

public:
	virtual TileMove getTileMove(const Tile * const tile, QList<Board::TilePlacement> const & placements, const Game * const game);
	virtual MeepleMove getMeepleMove(Tile const * const tile, QVarLengthArray<MeepleMove, NODE_ARRAY_LENGTH> const & possible, Game const * const game);
};

#endif // RANDOMPLAYER_H
