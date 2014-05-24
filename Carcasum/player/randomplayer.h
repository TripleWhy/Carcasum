#ifndef RANDOMPLAYER_H
#define RANDOMPLAYER_H

#include "core/player.h"
#include "core/random.h"
#include "core/game.h"

class RandomPlayer : public Player
{
public:
	static RandomPlayer instance;
	
private:
	RandomTable r;

public:
	RandomPlayer() = default;
	RandomPlayer(const RandomPlayer &) = delete;
	RandomPlayer & operator = (const RandomPlayer &) = delete;

	inline virtual void newGame(int /*player*/, Game const * /*game*/) {}
	inline virtual void playerMoved(int /*player*/, Tile const * /*tile*/, MoveHistoryEntry const & /*move*/) {}
	inline virtual TileMove getTileMove(int /*player*/, Tile const * /*tile*/, MoveHistoryEntry const & /*move*/, TileMovesType const & placements) { return placements[r.nextInt(placements.size())]; }
	inline virtual MeepleMove getMeepleMove(int /*player*/, Tile const * /*tile*/, MoveHistoryEntry const & /*move*/, MeepleMovesType const & possible) { return possible[r.nextInt(possible.size())]; }
	inline virtual void endGame() {}
	virtual QString getTypeName() const { return "RandomPlayer"; }
	virtual Player * clone() const { return new RandomPlayer(); }
};

#endif // RANDOMPLAYER_H
