#ifndef MONTECARLOPLAYER_H
#define MONTECARLOPLAYER_H

#include "static.h"
#include "utilities.h"
#include "playouts.h"
#include "core/game.h"
#include "core/player.h"
#include "jcz/tilefactory.h"

template<class UtilityProvider = Utilities::ComplexUtility, class Playout = Playouts::RandomPlayout>
class MonteCarloPlayer : public Player
{
	typedef typename UtilityProvider::RewardType RewardType;
	typedef typename UtilityProvider::RewardListType RewardListType;

private:
	Game const * game = 0;
	Game * simGame = 0;
	jcz::TileFactory * tileFactory;

	QString typeName;
	STATICCONSTEXPR Playout playoutPolicy = Playout();
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
	      typeName(QString("MonteCarloPlayer<%1, %2>").arg(UtilityProvider::name).arg(Playout::name)),
	      M(m),
		  useTimeout(mIsTimeout)
	{
	}

	virtual void newGame(int player, Game const * g);
	virtual void playerMoved(int player, Tile const * tile, MoveHistoryEntry const & move);
	virtual TileMove getTileMove(int player, Tile const * tile, MoveHistoryEntry const & move, TileMovesType const & placements);
	virtual MeepleMove getMeepleMove(int player, Tile const * tile, MoveHistoryEntry const & move, MeepleMovesType const & possible);
	virtual void endGame();
	virtual QString getTypeName() { return typeName; }
	virtual Player * clone() const;
	
private:
	inline RewardType utility(int const * scores, int const playerCount, int const myIndex, Game const * g)
	{
		return utilityProvider.utility(scores, playerCount, myIndex, g);
	}
};

#include "montecarloplayer.tpp"

#endif // MONTECARLOPLAYER_H
