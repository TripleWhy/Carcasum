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

#ifndef RANDOMPLAYER_H
#define RANDOMPLAYER_H

#include "core/player.h"
#include "core/random.h"
#include "core/game.h"

class RandomPlayer : public Player
{
public:
	static RandomPlayer instance;
	
private:
	RandomTable r;

public:
	RandomPlayer() = default;
	RandomPlayer(const RandomPlayer &) = delete;
	RandomPlayer & operator = (const RandomPlayer &) = delete;

	inline virtual void newGame(int /*player*/, Game const * /*game*/) {}
	inline virtual void playerMoved(int /*player*/, Tile const * /*tile*/, MoveHistoryEntry const & /*move*/) {}
	virtual void undoneMove(MoveHistoryEntry const & /*move*/) {}
	inline virtual TileMove getTileMove(int /*player*/, Tile const * /*tile*/, MoveHistoryEntry const & /*move*/, TileMovesType const & placements) { return placements[r.nextInt(placements.size())]; }
	inline virtual MeepleMove getMeepleMove(int /*player*/, Tile const * /*tile*/, MoveHistoryEntry const & /*move*/, MeepleMovesType const & possible) { return possible[r.nextInt(possible.size())]; }
	inline virtual void endGame() {}
	virtual QString getTypeName() const { return "RandomPlayer"; }
	virtual Player * clone() const { return new RandomPlayer(); }
};

#endif // RANDOMPLAYER_H
