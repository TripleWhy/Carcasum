#ifndef MONTECARLOPLAYER_H
#define MONTECARLOPLAYER_H

#include "static.h"
#include "core/game.h"
#include "core/player.h"
#include "jcz/tilefactory.h"

class MonteCarloPlayer : public Player
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
	bool useComplexUtility;
	
public:
	constexpr MonteCarloPlayer(jcz::TileFactory * tileFactory, bool useComplexUtility = true)
	    : tileFactory(tileFactory),
	      useComplexUtility(useComplexUtility)
	{
	}

//	~MonteCarloPlayer()
//	{
//		delete simGame;
//	}
	virtual void newGame(int player, Game const * g);
	virtual void playerMoved(int player, Tile const * tile, MoveHistoryEntry const & move);
	virtual TileMove getTileMove(int player, Tile const * tile, MoveHistoryEntry const & move, TileMovesType const & placements);
	virtual MeepleMove getMeepleMove(int player, Tile const * tile, MoveHistoryEntry const & move, MeepleMovesType const & possible);
	virtual void endGame();
	virtual char const * getTypeName() { return useComplexUtility ? "MonteCarloPlayer(complex)" : "MonteCarloPlayer(simple)"; }
	
private:
	inline static int utilitySimple(int const * scores, int const playerCount, int const myIndex)
	{
		return Util::utilitySimple(scores, playerCount, myIndex);
	}

	inline static int utilityComplex(int const * scores, int const playerCount, int const myIndex)
	{
		return Util::utilityComplex(scores, playerCount, myIndex);
	}

	typedef int (*utilityFunctionType)(int const *, int const, int const);
	constexpr utilityFunctionType utilityFunction()
	{
		return useComplexUtility ? &utilityComplex : &utilitySimple;
	}

	inline int utility(int const * scores, int const playerCount, int const myIndex)
	{
//		return utilityComplex(scores, playerCount, myIndex);
		return useComplexUtility ? utilityComplex(scores, playerCount, myIndex) : utilitySimple(scores, playerCount, myIndex);
//		return utilityFunction()(scores, playerCount, myIndex);
	}
};

#endif // MONTECARLOPLAYER_H
