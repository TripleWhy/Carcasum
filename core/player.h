#ifndef PLAYER_H
#define PLAYER_H

#include "tile.h"
#include "board.h"
#include "game.h"

#include <QList>

struct TileMove;
class Game;
class Tile;

class Player
{
public:
	virtual void newGame(Game const * const game) = 0;
	virtual void playerMoved(Tile const * const tile, TileMove const & tileMove, MeepleMove const & meepleMove, Game const * const game) = 0;
	virtual TileMove getTileMove(Tile const * const tile, QList<TileMove> const & placements, Game const * const game) = 0;
	virtual MeepleMove getMeepleMove(Tile const * const tile, QVarLengthArray<MeepleMove, NODE_ARRAY_LENGTH> const & possible, Game const * const game) = 0;
};

#endif // PLAYER_H
