#ifndef SIMPLEPLAYER_H
#define SIMPLEPLAYER_H

#include "core/player.h"
#include "core/game.h"
#include "core/board.h"
#include "core/random.h"

class SimplePlayer : public Player
{
private:
	Game const * game;
	Board const * board;
	RandomTable r;
	bool meepleMoveSet;
	MeepleMove meepleMove;

public:
	SimplePlayer() = default;
	SimplePlayer(const SimplePlayer &) = delete;
	SimplePlayer & operator = (const SimplePlayer &) = delete;

	inline virtual void newGame(int player, Game const * game);
	inline virtual void playerMoved(int player, Tile const * tile, MoveHistoryEntry const & move);
	inline virtual TileMove getTileMove(int player, Tile const * tile, MoveHistoryEntry const & move, TileMovesType const & placements);
	inline virtual MeepleMove getMeepleMove(int player, Tile const * tile, MoveHistoryEntry const & move, MeepleMovesType const & possible);
	inline virtual void endGame();
	virtual QString getTypeName();
	virtual Player * clone() const;
};

#endif // SIMPLEPLAYER_H
