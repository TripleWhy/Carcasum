#ifndef RANDOMPLAYER_H
#define RANDOMPLAYER_H

#include "core/player.h"
#include "core/util.h"

class RandomPlayer : public Player
{
private:
	Random r;

public:
	virtual Move getMove(const Tile * const /*tile*/, QList<Board::TilePlacement> const & placements, const Game * const game);
};

#endif // RANDOMPLAYER_H
