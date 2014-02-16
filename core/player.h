#ifndef PLAYER_H
#define PLAYER_H

#include "tile.h"
#include "board.h"
#include "game.h"

class Move;
class Game;
class Tile;

class Player
{
public:
	virtual Move getMove(Tile const * const tile, QList<Board::TilePlacement> const & placements, Game const * const game) = 0;
};

#endif // PLAYER_H
