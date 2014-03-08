#ifndef PLAYER_H
#define PLAYER_H

#include "game.h"

#include <QList>

class Game;
class Tile;

typedef QList<TileMove> TileMovesType;
typedef QVarLengthArray<MeepleMove, NODE_ARRAY_LENGTH> MeepleMovesType;

class Player
{
public:
	virtual void newGame(int player, Game const * const game) = 0;
	virtual void playerMoved(int player, Tile const * const tile, Move const & move, Game const * const game) = 0;
	virtual TileMove getTileMove(int player, Tile const * const tile, TileMovesType const & placements, Game const * const game) = 0;
	virtual MeepleMove getMeepleMove(int player, Tile const * const tile, MeepleMovesType const & possible, Game const * const game) = 0;
};

#endif // PLAYER_H
