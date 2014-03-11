#ifndef MONTECARLOPLAYER_H
#define MONTECARLOPLAYER_H

#include "core/player.h"
#include "jcz/tilefactory.h"

class MonteCarloPlayer : public Player
{
	static int const N = 100;
	
private:
	jcz::TileFactory * tileFactory;
	
public:
	MonteCarloPlayer(jcz::TileFactory * tileFactory);
	~MonteCarloPlayer();
	virtual void newGame(int player, Game const * const game);
	virtual void playerMoved(int player, Tile const * const tile, MoveHistoryEntry const & move, Game const * const game);
	virtual TileMove getTileMove(int player, Tile const * const tile, MoveHistoryEntry const & move, TileMovesType const & placements, Game const * const game);
	virtual MeepleMove getMeepleMove(int player, Tile const * const tile, MoveHistoryEntry const & move, MeepleMovesType const & possible, Game const * const game);
	
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
