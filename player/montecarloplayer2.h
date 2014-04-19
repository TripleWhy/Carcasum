#ifndef MONTECARLOPLAYER2_H
#define MONTECARLOPLAYER2_H

#include "static.h"
#include "core/game.h"
#include "core/player.h"
#include "jcz/tilefactory.h"

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
	int useComplexUtility;
	MeepleMove meepleMove;
	static RandomTable r;

public:
	constexpr MonteCarloPlayer2(jcz::TileFactory * tileFactory, int useComplexUtility = 1)
	    : tileFactory(tileFactory),
	      useComplexUtility(useComplexUtility)
	{
	}

	virtual void newGame(int player, Game const * g);
	virtual void playerMoved(int player, Tile const * tile, MoveHistoryEntry const & move);
	virtual TileMove getTileMove(int player, Tile const * tile, MoveHistoryEntry const & move, TileMovesType const & placements);
	virtual MeepleMove getMeepleMove(int player, Tile const * tile, MoveHistoryEntry const & move, MeepleMovesType const & possible);
	virtual void endGame();
	virtual char const * getTypeName() { return useComplexUtility ? "MonteCarloPlayer2(complex)" : "MonteCarloPlayer2(simple)"; }

private:
	int playout();
	void unplayout(int steps);

	inline int utility(int const * scores, int const playerCount, int const myIndex)
	{
//		return useComplexUtility ? utilityComplex(scores, playerCount, myIndex) : utilitySimple(scores, playerCount, myIndex);
		switch (useComplexUtility)
		{
			case 0:
				return Util::utilitySimple(scores, playerCount, myIndex);
			case 1:
				return Util::utilityComplex(scores, playerCount, myIndex);
			case 2:
				return Util::utilityComplexOld(scores, playerCount, myIndex);
			default:
				return 0;
		}
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

#endif // MONTECARLOPLAYER2_H
