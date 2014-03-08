#ifndef RANDOMPLAYER_H
#define RANDOMPLAYER_H

#include "core/player.h"
#include "core/util.h"

class RandomPlayer : public Player
{
public:
	static RandomPlayer instance;
	
private:
	Random r;

public:
	inline virtual void newGame(int /*player*/, Game const * const /*game*/) {}
	inline virtual void playerMoved(int /*player*/, Tile const * const /*tile*/, Move const & /*move*/, Game const * const /*game*/) {}
	inline virtual TileMove getTileMove(int /*player*/, const Tile * const /*tile*/, TileMovesType const & placements, const Game * const /*game*/) { return placements[r.nextInt(placements.size())]; }
	inline virtual MeepleMove getMeepleMove(int /*player*/, Tile const * const /*tile*/, MeepleMovesType const & possible, Game const * const /*game*/) { return possible[r.nextInt(possible.size())]; }
};

#endif // RANDOMPLAYER_H
