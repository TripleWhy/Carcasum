#ifndef RANDOMPLAYER_H
#define RANDOMPLAYER_H

#include "core/player.h"
#include "core/util.h"
#include "core/game.h"

class RandomPlayer : public Player
{
public:
	static RandomPlayer instance;
	
private:
	Random r;

public:
	inline virtual void newGame(int /*player*/, Game const * const /*game*/) {}
	inline virtual void playerMoved(int /*player*/, Tile const * const /*tile*/, MoveHistoryEntry const & /*move*/, Game const * const /*game*/) {}
	inline virtual TileMove getTileMove(int /*player*/, Tile const * const /*tile*/, MoveHistoryEntry const & /*move*/, TileMovesType const & placements, Game const * const /*game*/) { return placements[r.nextInt(placements.size())]; }
	inline virtual MeepleMove getMeepleMove(int /*player*/, Tile const * const /*tile*/, MoveHistoryEntry const & /*move*/, MeepleMovesType const & possible, Game const * const /*game*/) { return possible[r.nextInt(possible.size())]; }
    inline virtual void endGame(Game const * const /*game*/) {}
};

#endif // RANDOMPLAYER_H
