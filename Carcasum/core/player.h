#ifndef PLAYER_H
#define PLAYER_H

#include "static.h"
#include <QString>

class Game;
class Tile;
struct TileMove;
struct MeepleMove;
struct MoveHistoryEntry;

typedef VarLengthArrayWrapper<TileMove, TILE_ARRAY_LENGTH>::type TileMovesType;
typedef VarLengthArrayWrapper<MeepleMove, NODE_ARRAY_LENGTH>::type MeepleMovesType;

class Player
{
#if COUNT_PLAYOUTS
public:
	int playouts = 0;
#endif

public:
	Player() = default;
	Player (Player &&) = default;
	Player (const Player &) = delete;
	Player & operator = (const Player &) = delete;

	virtual ~Player() {}
	virtual void newGame(int player, Game const * game) = 0;
	virtual void playerMoved(int player, Tile const * tile, MoveHistoryEntry const & move) = 0;
	virtual TileMove getTileMove(int player, Tile const * tile, MoveHistoryEntry const & move, TileMovesType const & placements) = 0;
	virtual MeepleMove getMeepleMove(int player, Tile const * tile, MoveHistoryEntry const & move, MeepleMovesType const & possible) = 0;
	virtual void endGame() = 0;
	virtual QString getTypeName() = 0;
	virtual Player * clone() const = 0;
};

#endif // PLAYER_H
