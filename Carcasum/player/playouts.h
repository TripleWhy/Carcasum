#ifndef PLAYOUTS_H
#define PLAYOUTS_H

#include "randomplayer.h"
#include "core/tile.h"
#include "core/game.h"
#include "player/simpleplayer2.h"

namespace Playouts
{

class RandomPlayout
{
public:
	constexpr static char const * name = "RandomPlayout";
	constexpr RandomPlayout() {}
	inline void newGame(int /*player*/, Game const * /*g*/) {}
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
			simGame.simUndo();
	}
};

template<int Cutoff = 16>
class EarlyCutoff : public RandomPlayout
{
public:
	static const QString name;
	constexpr EarlyCutoff() {}
	inline void newGame(int /*player*/, Game const * /*g*/) {}
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
		Q_ASSERT(simGame.isFinished());
		if (!simGame.isTerminal())
			simGame.simUnEndGame();
		RandomPlayout::undoPlayout(simGame, steps);
	}
};
template<int Cutoff>
QString const Playouts::EarlyCutoff<Cutoff>::name = QString("EarlyCutoff<%1>").arg(Cutoff);


template<int RndPercent, typename Pl = SimplePlayer2>
class EGreedyPlayer
{
private:
	Pl m_player;
	RandomTable r;
	int magic = -379;
public:
	const QString name = QString("EGreedyPlayer<%1, %2>").arg(RndPercent).arg(m_player.getTypeName());
	constexpr EGreedyPlayer() {}
	constexpr EGreedyPlayer(EGreedyPlayer &&) = default;
	inline void newGame(int player, Game const * g)
	{
		m_player.newGame(player, g);
	}
	inline TileMove chooseTileMove(int player, Tile const * tile, MoveHistoryEntry const & move, TileMovesType const & possibleTiles)
	{
		if (RndPercent >= 0 && (RndPercent >= 100 || (r.nextInt(100) < RndPercent)) )
			return RandomPlayer::instance.getTileMove(player, tile, move, possibleTiles);
		else
			return m_player.getTileMove(player, tile, move, possibleTiles);
	}
	inline MeepleMove chooseMeepleMove(int player, Tile const * tile, MoveHistoryEntry const & move, MeepleMovesType const & possibleMeeples)
	{
		if (RndPercent >= 0 && (RndPercent >= 100 || (r.nextInt(100) < RndPercent)) )
			return RandomPlayer::instance.getMeepleMove(player, tile, move, possibleMeeples);
		else
			return m_player.getMeepleMove(player, tile, move, possibleMeeples);
	}
	inline int playout(Game & simGame)
	{
		int steps = 0;
		if (!simGame.isFinished())
		{
			bool cont;
			do
			{
				++steps;
				if (RndPercent >= 0 && (RndPercent >= 100 || (r.nextInt(100) < RndPercent)) )
					cont = simGame.simStep(&RandomPlayer::instance);
				else
					cont = simGame.simStep(&m_player);
			} while (cont);
		}
		return steps;
	}
	inline void undoPlayout(Game & simGame, int steps) const
	{
		for (int i = 0; i < steps; ++i)
			simGame.simUndo();
	}
};


template<int RndPercent>
class EGreedy1Ply : public RandomPlayout
{
private:
	RandomTable r;
public:
	static const QString name;
	constexpr EGreedy1Ply() {}
	inline int playout(Game & simGame)
	{
		int steps = 0;
		if (!simGame.isFinished())
		{
			bool cont;
			do
			{
				++steps;
				if (RndPercent >= 0 && (RndPercent >= 100 || (r.nextInt(100) < RndPercent)) )
				{
					cont = simGame.simStep(&RandomPlayer::instance);
				}
				else
				{
					int const player = simGame.getNextPlayer();
					int const tileCount = simGame.getTileCount();

					int bestScore = -1;
					MoveHistoryEntry best;

					best.tileIndex = r.nextInt(tileCount);
					simGame.simPartStepChance(best.tileIndex);
					Tile const * tile = simGame.simTile;
					auto const & possibleTiles = simGame.getPossibleTilePlacements(tile);

					//select best move
					for (auto const & tm : possibleTiles)
					{
						simGame.simPartStepTile(tm);

						auto const & possibleMeeples = simGame.getPossibleMeeplePlacements(player, tile);
						for (auto const & mm : possibleMeeples)
						{
							simGame.simPartStepMeeple(mm);
							simGame.simEndGame();

							int const score = simGame.getPlayerScore(player);
							if (score > bestScore)
							{
								bestScore = score;
								best.move.tileMove = tm;
								best.move.meepleMove = mm;
							}

							simGame.simUnEndGame();
							simGame.simPartUndoMeeple();
						}

						simGame.simPartUndoTile();
					}
					simGame.simPartUndoChance();

					//simulate selected move
					cont = simGame.simStep(best);
				}

			} while (cont);
		}
		return steps;
	}
};
template<int RndPercent>
QString const Playouts::EGreedy1Ply<RndPercent>::name = QString("EGreedy1Ply<%1>").arg(RndPercent);

}

#endif // PLAYOUTS_H
