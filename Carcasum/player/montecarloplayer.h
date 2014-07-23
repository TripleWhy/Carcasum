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

#ifndef MONTECARLOPLAYER_H
#define MONTECARLOPLAYER_H

#include "static.h"
#include "utilities.h"
#include "playouts.h"
#include "core/game.h"
#include "core/player.h"
#include "jcz/tilefactory.h"

template<class UtilityProvider = Utilities::PortionUtility, class Playout = Playouts::RandomPlayout>
class MonteCarloPlayer : public Player
{
	typedef typename UtilityProvider::RewardType RewardType;
	typedef typename UtilityProvider::RewardListType RewardListType;

private:
	Game const * game = 0;
	Game * simGame = 0;
	jcz::TileFactory * tileFactory;

	QString typeName;
	Playout playoutPolicy;
	UtilityProvider utilityProvider = UtilityProvider();
	const int M;
	const bool useTimeout;

public:
#ifdef TIMEOUT
	constexpr MonteCarloPlayer(jcz::TileFactory * tileFactory, int m = TIMEOUT, bool mIsTimeout = true)
#else
	constexpr MonteCarloPlayer(jcz::TileFactory * tileFactory, int m = 5000, bool mIsTimeout = true)
#endif
	    : tileFactory(tileFactory),
	      typeName(QString("MonteCarloPlayer<%1, %2>(m=%4, mIsTimeout=%5)").arg(UtilityProvider::name).arg(Playout::name).arg(m).arg(mIsTimeout)),
	      M(m),
		  useTimeout(mIsTimeout)
	{
	}

	virtual void newGame(int player, Game const * g);
	virtual void playerMoved(int player, Tile const * tile, MoveHistoryEntry const & move);
	virtual void undoneMove(MoveHistoryEntry const & move);
	virtual TileMove getTileMove(int player, Tile const * tile, MoveHistoryEntry const & move, TileMovesType const & placements);
	virtual MeepleMove getMeepleMove(int player, Tile const * tile, MoveHistoryEntry const & move, MeepleMovesType const & possible);
	virtual void endGame();
	virtual QString getTypeName() const { return typeName; }
	virtual Player * clone() const;
	
private:
	inline RewardType utility(int const * scores, int const playerCount, int const myIndex, Game const * g)
	{
		return utilityProvider.utility(scores, playerCount, myIndex, g);
	}
};

#include "montecarloplayer.tpp"

#endif // MONTECARLOPLAYER_H
