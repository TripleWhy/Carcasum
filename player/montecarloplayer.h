#ifndef MONTECARLOPLAYER_H
#define MONTECARLOPLAYER_H

#include "static.h"
#include "core/game.h"
#include "core/player.h"
#include "jcz/tilefactory.h"

class MonteCarloPlayer : public Player
{
	static int const N = 100;
#if COUNT_PLAYOUTS
public:
	int playouts = 0;
#endif
	
private:
	Game const * game;
	Game simGame = Game(0);
	jcz::TileFactory * tileFactory;
	
public:
	MonteCarloPlayer(jcz::TileFactory * tileFactory);
	~MonteCarloPlayer();
	virtual void newGame(int player, Game const * g);
	virtual void playerMoved(int player, Tile const * tile, MoveHistoryEntry const & move);
	virtual TileMove getTileMove(int player, Tile const * tile, MoveHistoryEntry const & move, TileMovesType const & placements);
	virtual MeepleMove getMeepleMove(int player, Tile const * tile, MoveHistoryEntry const & move, MeepleMovesType const & possible);
	virtual void endGame();
	virtual char const * getTypeName() { return "MonteCarloPlayer"; }
	
private:
	inline int utility(int const * scores, int const playerCount, int const myIndex)
	{
		std::map<int, int> map;
		for (int i = 0; i < playerCount; ++i)
			map[scores[i]] = i;
		
		int u = 0;
		int m = 0;
		int lastScore = -1;
		// last player first
		for (auto const & e : map)
		{
			if (e.first != lastScore)
				++m;
			if (e.second == myIndex)
				u += m * e.first;
			else
				u -= m * e.first;
			lastScore = e.first;
		}
		
		return u;
	}
};

#endif // MONTECARLOPLAYER_H
