#ifndef METAPLAYERS_H
#define METAPLAYERS_H

#include "core/player.h"
#include "core/game.h"
#include "core/random.h"
#include "player/simpleplayer3.h"
#include "core/util.h"

template<int RndPercent, typename Pl = SimplePlayer3>
class EGreedyPlayer : public Player
{
private:
	Pl m_player;
	RandomTable r;
	const QString name = QString("EGreedyPlayer<%1, %2>").arg(RndPercent).arg(m_player.getTypeName());

public:
	constexpr EGreedyPlayer() {}
	constexpr EGreedyPlayer(EGreedyPlayer &&) = default;

	virtual void newGame(int player, Game const * game);
	virtual void playerMoved(int player, Tile const * tile, MoveHistoryEntry const & move);
	virtual void undoneMove(MoveHistoryEntry const & move);
	virtual TileMove getTileMove(int player, Tile const * tile, MoveHistoryEntry const & move, TileMovesType const & possible);
	virtual MeepleMove getMeepleMove(int player, Tile const * tile, MoveHistoryEntry const & move, MeepleMovesType const & possible);
	virtual void endGame();
	virtual QString getTypeName() const;
	virtual Player * clone() const;
};

class RouletteWheelPlayer : public Player
{
private:
	RandomTable r;
	Game const * game;
	MeepleMove meepleMove;

public:
	virtual void newGame(int player, Game const * game);
	virtual void playerMoved(int player, Tile const * tile, MoveHistoryEntry const & move);
	virtual void undoneMove(MoveHistoryEntry const & move);
	virtual TileMove getTileMove(int player, Tile const * tile, MoveHistoryEntry const & move, TileMovesType const & possible);
	virtual MeepleMove getMeepleMove(int player, Tile const * tile, MoveHistoryEntry const & move, MeepleMovesType const & possible);
	virtual void endGame();
	virtual QString getTypeName() const;
	virtual Player * clone() const;
};

class RouletteWheelPlayer2 : public Player
{
private:
	RandomTable r;
	Game const * game;
	SimplePlayer3::RatingsNMeepleType meepleRatings;
	int meepleSum;

public:
	virtual void newGame(int player, Game const * game);
	virtual void playerMoved(int player, Tile const * tile, MoveHistoryEntry const & move);
	virtual void undoneMove(MoveHistoryEntry const & move);
	virtual TileMove getTileMove(int player, Tile const * tile, MoveHistoryEntry const & move, TileMovesType const & possible);
	virtual MeepleMove getMeepleMove(int player, Tile const * tile, MoveHistoryEntry const & move, MeepleMovesType const & possible);
	virtual void endGame();
	virtual QString getTypeName() const;
	virtual Player * clone() const;
};

template<typename Utility>
class UtilityPlayer1 : public Player
{
private:
	Utility utility;
	Game const * game;
	Game simGame = Game(0);
	const QString name = QString("UtilityPlayer1<%1>").arg(utility.name);
	jcz::TileFactory * tileFactory;

public:
	UtilityPlayer1(jcz::TileFactory * tf) : tileFactory(tf) {}
	virtual void newGame(int player, Game const * g)
	{
		endGame();
		game = g;
		Util::setupNewGame(*game, simGame, tileFactory);
		utility.newGame(player, game);
	}
	virtual void playerMoved(int /*player*/, Tile const * /*tile*/, MoveHistoryEntry const & /*move*/)
	{
		Util::syncGamesFast(*game, simGame);
	}
	virtual void undoneMove(MoveHistoryEntry const & /*move*/)
	{
		Util::syncGames(*game, simGame);
	}
	virtual TileMove getTileMove(int player, Tile const * /*tile*/, MoveHistoryEntry const & move, TileMovesType const & possible)
	{
		TileMove const * best = 0;
		auto bestU = std::numeric_limits<typename Utility::RewardType>::lowest();

		Q_ASSERT(game->equals(simGame));
		simGame.simPartStepChance(move.tileIndex);
		for (TileMove const & tm : possible)
		{
			simGame.simPartStepTile(tm);
			auto u = utility.utility(simGame.getScores(), simGame.getPlayerCount(), player, &simGame);
			if (u > bestU)
			{
				bestU = u;
				best = &tm;
			}
			simGame.simPartUndoTile();
		}
		simGame.simPartUndoChance();
		Q_ASSERT(game->equals(simGame));

		Q_ASSERT(best != 0);
		return *best;
	}
	virtual MeepleMove getMeepleMove(int player, Tile const * /*tile*/, MoveHistoryEntry const & move, MeepleMovesType const & possible)
	{
		MeepleMove const * best = 0;
		auto bestU = std::numeric_limits<typename Utility::RewardType>::lowest();

		simGame.simPartStepChance(move.tileIndex);
		simGame.simPartStepTile(move.move.tileMove);
		Q_ASSERT(game->equals(simGame));
		for (MeepleMove const & mm : possible)
		{
			simGame.simPartStepMeeple(mm);
			auto u = utility.utility(simGame.getScores(), simGame.getPlayerCount(), player, &simGame);
			if (u > bestU)
			{
				bestU = u;
				best = &mm;
			}
			simGame.simPartUndoMeeple();
		}
//		Q_ASSERT(game->equals(simGame));
		simGame.simPartUndoTile();
		simGame.simPartUndoChance();

		Q_ASSERT(best != 0);
		return *best;
	}
	virtual void endGame()
	{
	}
	virtual QString getTypeName() const
	{
		return name;
	}
	virtual Player * clone() const
	{
		return new UtilityPlayer1(tileFactory);
	}
};

template<typename Utility>
class UtilityPlayer2 : public Player
{
private:
	Utility utility;
	Game const * game;
	Game simGame = Game(0);
	const QString name = QString("UtilityPlayer2<%1>").arg(utility.name);
	MeepleMove meepleMove;
	jcz::TileFactory * tileFactory;

public:
	UtilityPlayer2(jcz::TileFactory * tf) : tileFactory(tf) {}
	virtual void newGame(int player, Game const * g)
	{
		endGame();
		game = g;
		Util::setupNewGame(*game, simGame, tileFactory);
		utility.newGame(player, game);
	}
	virtual void playerMoved(int /*player*/, Tile const * /*tile*/, MoveHistoryEntry const & /*move*/)
	{
		Util::syncGamesFast(*game, simGame);
	}
	virtual void undoneMove(MoveHistoryEntry const & /*move*/)
	{
		Util::syncGames(*game, simGame);
	}
	virtual TileMove getTileMove(int player, Tile const * /*tile*/, MoveHistoryEntry const & move, TileMovesType const & possible)
	{
		TileMove const * best = 0;
		auto bestU = std::numeric_limits<typename Utility::RewardType>::lowest();

		simGame.simPartStepChance(move.tileIndex);
		for (TileMove const & tm : possible)
		{
			simGame.simPartStepTile(tm);

			for (MeepleMove const & mm : simGame.getPossibleMeeplePlacements(player, simGame.simTile))
			{
				simGame.simPartStepMeeple(mm);
				auto u = utility.utility(simGame.getScores(), simGame.getPlayerCount(), player, &simGame);
				if (u > bestU)
				{
					bestU = u;
					best = &tm;
					meepleMove = mm;
				}
				simGame.simPartUndoMeeple();
			}

			simGame.simPartUndoTile();
		}
		simGame.simPartUndoChance();

		Q_ASSERT(best != 0);
		return *best;
	}
	virtual MeepleMove getMeepleMove(int /*player*/, Tile const * /*tile*/, MoveHistoryEntry const & /*move*/, MeepleMovesType const & /*possible*/)
	{
		return meepleMove;
	}
	virtual void endGame()
	{
	}
	virtual QString getTypeName() const
	{
		return name;
	}
	virtual Player * clone() const
	{
		return new UtilityPlayer2(tileFactory);
	}
};

#endif // METAPLAYERS_H
