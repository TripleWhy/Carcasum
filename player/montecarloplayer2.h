#ifndef MONTECARLOPLAYER2_H
#define MONTECARLOPLAYER2_H

#include "static.h"
#include "playouts.h"
#include "utilities.h"
#include "core/game.h"
#include "core/player.h"
#include "jcz/tilefactory.h"

template<class UtilityProvider = Utilities::ComplexUtility, class Playout = Playouts::RandomPlayout>
class MonteCarloPlayer2 : public Player
{
#ifndef TIMEOUT
 #ifdef QT_NO_DEBUG
	static int const N = 100;
 #else
	static int const N = 20;
 #endif
#endif

private:
	Game const * game = 0;
	Game * simGame = 0;
	jcz::TileFactory * tileFactory;
	MeepleMove meepleMove;
	static RandomTable r;

	QString typeName;
	STATICCONSTEXPR Playout playoutPolicy = Playout();
	UtilityProvider utilityProvider = UtilityProvider();

//public:
//	int min = std::numeric_limits<int>::max();
//	int max = std::numeric_limits<int>::min();

public:
	constexpr MonteCarloPlayer2(jcz::TileFactory * tileFactory)
	    : tileFactory(tileFactory),
	      typeName(QString("MonteCarloPlayer2<%1, %2>").arg(UtilityProvider::name).arg(Playout::name))
	{
	}

	virtual void newGame(int player, Game const * g);
	virtual void playerMoved(int player, Tile const * tile, MoveHistoryEntry const & move);
	virtual TileMove getTileMove(int player, Tile const * tile, MoveHistoryEntry const & move, TileMovesType const & placements);
	virtual MeepleMove getMeepleMove(int player, Tile const * tile, MoveHistoryEntry const & move, MeepleMovesType const & possible);
	virtual void endGame();
	virtual QString getTypeName() { return typeName; }

private:
	int playout();
	void unplayout(int steps);

	inline int utility(int const * scores, int const playerCount, int const myIndex)
	{
		return utilityProvider.utility(scores, playerCount, myIndex);
	}

	inline int chooseTileMove(TileMovesType const & possible)
	{
		return r.nextInt(possible.size());
	}

	inline int chooseMeepleMove(MeepleMovesType const & possible)
	{
		return r.nextInt(possible.size());
	}
};

#include "montecarloplayer2.tpp"

#endif // MONTECARLOPLAYER2_H
