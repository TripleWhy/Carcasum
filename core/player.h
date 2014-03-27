#ifndef PLAYER_H
#define PLAYER_H

#include "static.h"
#include <QVarLengthArray>

#include <QList>

class Game;
class Tile;
struct TileMove;
struct MeepleMove;
struct MoveHistoryEntry;

typedef QVarLengthArray<TileMove, TILE_ARRAY_LENGTH> TileMovesType;
typedef QVarLengthArray<MeepleMove, NODE_ARRAY_LENGTH> MeepleMovesType;

class Player
{
public:
	virtual void newGame(int player, Game const * game) = 0;
	virtual void playerMoved(int player, Tile const * tile, MoveHistoryEntry const & move) = 0;
	virtual TileMove getTileMove(int player, Tile const * tile, MoveHistoryEntry const & move, TileMovesType const & placements) = 0;
	virtual MeepleMove getMeepleMove(int player, Tile const * tile, MoveHistoryEntry const & move, MeepleMovesType const & possible) = 0;
	virtual void endGame() = 0;
};

#endif // PLAYER_H
