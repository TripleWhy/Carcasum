#ifndef SIMPLEPLAYER2_H
#define SIMPLEPLAYER2_H

#include "core/player.h"
#include "core/game.h"
#include "core/board.h"
#include "core/random.h"

class SimplePlayer2 : public Player
{
private:
	Game const * game;
	Board const * board;
	RandomTable r;
	bool meepleMoveSet;
	MeepleMove meepleMove;

public:
	SimplePlayer2() = default;
	SimplePlayer2(const SimplePlayer2 &) = delete;
	SimplePlayer2 & operator = (const SimplePlayer2 &) = delete;

	inline virtual void newGame(int player, Game const * game);
	inline virtual void playerMoved(int player, Tile const * tile, MoveHistoryEntry const & move);
	inline virtual TileMove getTileMove(int player, Tile const * tile, MoveHistoryEntry const & move, TileMovesType const & placements);
	inline virtual MeepleMove getMeepleMove(int player, Tile const * tile, MoveHistoryEntry const & move, MeepleMovesType const & possible);
	inline virtual void endGame();
	virtual QString getTypeName();
	virtual Player * clone() const;
};

#endif // SIMPLEPLAYER2_H
