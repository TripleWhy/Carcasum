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
	virtual TileMove getTileMove(Tile const * const tile, QList<Board::TilePlacement> const & placements, Game const * const game) = 0;
	virtual MeepleMove getMeepleMove(Tile const * const tile, QVarLengthArray<MeepleMove, NODE_ARRAY_LENGTH> const & possible, Game const * const game) = 0;
};

#endif // PLAYER_H
