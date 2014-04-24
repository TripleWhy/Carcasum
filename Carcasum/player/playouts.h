#ifndef PLAYOUTS_H
#define PLAYOUTS_H

#include "randomplayer.h"
#include "core/tile.h"
#include "core/game.h"

namespace Playouts
{

class RandomPlayout
{
public:
	constexpr static char const * name = "RandomPlayout";
	constexpr RandomPlayout() {}
	inline TileMove chooseTileMove(int player, Tile const * tile, MoveHistoryEntry const & move, TileMovesType const & possibleTiles) const
	{
		return RandomPlayer::instance.getTileMove(player, tile, move, possibleTiles);
	}
	inline MeepleMove chooseMeepleMove(int player, Tile const * tile, MoveHistoryEntry const & move, MeepleMovesType const & possibleMeeples) const
	{
		return RandomPlayer::instance.getMeepleMove(player, tile, move, possibleMeeples);
	}
	inline int playout(Game & simGame) const
	{
		int steps = 0;
		if (!simGame.isFinished())
		{
			do
				++steps;
			while (simGame.simStep(&RandomPlayer::instance));
		}
		return steps;
	}
	inline void undoPlayout(Game & simGame, int steps) const
	{
		for (int i = 0; i < steps; ++i)
			simGame.undo();
	}
};

template<int Cutoff>
class EarlyCutoff : public RandomPlayout
{
private:
public:
	static const QString name;
	constexpr EarlyCutoff() {}
	inline int playout(Game & simGame) const
	{
		int steps = 0;
		if (!simGame.isFinished())
		{
			bool cont;
			forever
			{
				++steps;
				cont = simGame.simStep(&RandomPlayer::instance);
				if (cont)
				{
					if (steps >= Cutoff)
					{
						simGame.simEndGame();
						break;
					}
				}
				else
					break;
			}
		}
		return steps;
	}
	inline void undoPlayout(Game & simGame, int steps) const
	{
		if (simGame.isFinished() && simGame.getTileCount() != 0)
			simGame.simUnEndGame();
		RandomPlayout::undoPlayout(simGame, steps);
	}
};

template<int Cutoff>
QString const Playouts::EarlyCutoff<Cutoff>::name = QString("EarlyCutoff<%1>").arg(Cutoff);

}

#endif // PLAYOUTS_H
