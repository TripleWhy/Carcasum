#ifndef RANDOMPLAYER_H
#define RANDOMPLAYER_H

#include "core/player.h"
#include "core/util.h"

class RandomPlayer : public Player
{
private:
	Random r;

public:
	virtual void newGame(int /*player*/, Game const * const /*game*/) {}
	virtual void playerMoved(int /*player*/, Tile const * const /*tile*/, TileMove const & /*tileMove*/, MeepleMove const & /*meepleMove*/, Game const * const /*game*/) {}
	virtual TileMove getTileMove(int player, const Tile * const tile, QList<TileMove> const & placements, const Game * const game);
	virtual MeepleMove getMeepleMove(int player, Tile const * const tile, QVarLengthArray<MeepleMove, NODE_ARRAY_LENGTH> const & possible, Game const * const game);
};

#endif // RANDOMPLAYER_H
