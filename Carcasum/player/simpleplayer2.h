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
	virtual void newGame(int player, Game const * game);
	virtual void playerMoved(int player, Tile const * tile, MoveHistoryEntry const & move);
	virtual void undoneMove(MoveHistoryEntry const & move);
	virtual TileMove getTileMove(int player, Tile const * tile, MoveHistoryEntry const & move, TileMovesType const & placements);
	virtual MeepleMove getMeepleMove(int player, Tile const * tile, MoveHistoryEntry const & move, MeepleMovesType const & possible);
	virtual void endGame();
	virtual QString getTypeName() const;
	virtual Player * clone() const;
};

#endif // SIMPLEPLAYER2_H
