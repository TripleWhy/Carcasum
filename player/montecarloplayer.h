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
#ifndef TIMEOUT
 #ifdef QT_NO_DEBUG
	static int const N = 100;
 #else
	static int const N = 20;
 #endif
#endif

	typedef typename UtilityProvider::RewardType RewardType;
	typedef typename UtilityProvider::RewardListType RewardListType;

private:
	Game const * game = 0;
	Game * simGame = 0;
	jcz::TileFactory * tileFactory;

	QString typeName;
	STATICCONSTEXPR Playout playoutPolicy = Playout();
	UtilityProvider utilityProvider = UtilityProvider();
	
public:
	constexpr MonteCarloPlayer(jcz::TileFactory * tileFactory)
	    : tileFactory(tileFactory),
	      typeName(QString("MonteCarloPlayer<%1, %2>").arg(UtilityProvider::name).arg(Playout::name))
	{
	}

	virtual void newGame(int player, Game const * g);
	virtual void playerMoved(int player, Tile const * tile, MoveHistoryEntry const & move);
	virtual TileMove getTileMove(int player, Tile const * tile, MoveHistoryEntry const & move, TileMovesType const & placements);
	virtual MeepleMove getMeepleMove(int player, Tile const * tile, MoveHistoryEntry const & move, MeepleMovesType const & possible);
	virtual void endGame();
	virtual QString getTypeName() { return typeName; }
	
private:
	inline int utility(int const * scores, int const playerCount, int const myIndex)
	{
		return utilityProvider.utility(scores, playerCount, myIndex);
	}
};

#include "montecarloplayer.tpp"

#endif // MONTECARLOPLAYER_H
