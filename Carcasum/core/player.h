/*
	This file is part of Carcasum.

	Carcasum is free software: you can redistribute it and/or modify
	it under the terms of the GNU Affero General Public License as published by
	the Free Software Foundation, either version 3 of the License, or
	(at your option) any later version.

	Carcasum is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU Affero General Public License for more details.

	You should have received a copy of the GNU Affero General Public License
	along with Carcasum.  If not, see <http://www.gnu.org/licenses/>.
*/

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
	virtual void undoneMove(MoveHistoryEntry const & move) = 0;
	virtual TileMove getTileMove(int player, Tile const * tile, MoveHistoryEntry const & move, TileMovesType const & placements) = 0;
	virtual MeepleMove getMeepleMove(int player, Tile const * tile, MoveHistoryEntry const & move, MeepleMovesType const & possible) = 0;
	virtual void endGame() = 0;
	virtual QString getTypeName() const = 0;
	virtual Player * clone() const = 0;
};

#endif // PLAYER_H
