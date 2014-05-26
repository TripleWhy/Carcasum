#include "montecarloplayer.h"
#include "randomplayer.h"
#include <QElapsedTimer>

#define MC_T template<class UtilityProvider, class Playout>
#define MC_TU <UtilityProvider, Playout>

MC_T
void MonteCarloPlayer MC_TU::newGame(int player, const Game * g)
{
	if (simGame == 0)
		simGame = new Game(0);
	game = g;
	simGame->clearPlayers();
	for (uint i = 0; i < g->getPlayerCount(); ++i)
		simGame->addPlayer(&RandomPlayer::instance);
	simGame->newGame(game->getTileSets(), tileFactory, g->getMoveHistory());
	utilityProvider.newGame(player, g);
	playoutPolicy.newGame(player, g);
}

MC_T
void MonteCarloPlayer MC_TU::playerMoved(int /*player*/, const Tile * /*tile*/, const MoveHistoryEntry & /*move*/)
{
	Util::syncGamesFast(*game, *simGame);
}

MC_T
void MonteCarloPlayer MC_TU::undoneMove(const MoveHistoryEntry & /*move*/)
{
	Util::syncGames(*game, *simGame);
}

MC_T
TileMove MonteCarloPlayer MC_TU::getTileMove(int player, const Tile * /*tile*/, const MoveHistoryEntry & move, const TileMovesType & possible)
{
	QElapsedTimer timer;
	if (useTimeout)
		timer.start();

	Util::syncGamesFast(*game, *simGame);

	int const playerCount = game->getPlayerCount();
	int const possibleSize = possible.size();
	auto utilities = QVarLengthArray<RewardType, 128>(possibleSize);
	for (int i = 0; i < possibleSize; ++i)
		utilities[i] = 0;

	Q_ASSERT(game->equals(*simGame));
	
	int i = 0;
	do
	{
		int moveIndex = 0;
		for (TileMove const & tileMove : possible)
		{
#if USE_RESET
// This branch is out of date.
			g.restartGame(history);
			Q_ASSERT(game->equals(g));
			g.step(move.tile, tileMove, player, &RandomPlayer::instance);

			while (!g.isFinished())
				g.simStep(&RandomPlayer::instance);

			utilities[moveIndex] += utility(g.getScores(), playerCount, player);
#else
//			simGame->simStep(move.tile, tileMove, player, &RandomPlayer::instance);
			// Hm, this looks a bit ugly.
			simGame->simPartStepChance(move.tileIndex);
			simGame->simPartStepTile(tileMove);
			MeepleMove meepleMove;
			if (simGame->getPlayerMeeples(player) > 0)
				meepleMove = playoutPolicy.chooseMeepleMove(player, simGame->simTile, simGame->simEntry, simGame->getPossibleMeeplePlacements(simGame->simTile));
			simGame->simPartStepMeeple(meepleMove);
			int steps = 1 + playoutPolicy.playout(*simGame);
			
			utilities[moveIndex] += utility(simGame->getScores(), playerCount, player, simGame);
			
			playoutPolicy.undoPlayout(*simGame, steps);
			Q_ASSERT(game->equals(*simGame));
#endif
			++moveIndex;
		}

		if (!useTimeout)
			i += possibleSize;
#if COUNT_PLAYOUTS
		else
			playouts += possibleSize;
#endif
	} while (useTimeout ? !timer.hasExpired((2*M)/3) : i < M);

#if COUNT_PLAYOUTS
	if (!useTimeout)
		playouts += i;
#endif
	
	TileMove const * bestMove = 0;
	RewardType bestUtility = std::numeric_limits<RewardType>::has_infinity ? (-std::numeric_limits<RewardType>::infinity()) : std::numeric_limits<RewardType>::lowest();
	for (int i = 0; i < possibleSize; ++i)
	{
		auto u = utilities[i];
		if (u > bestUtility)
		{
			bestUtility = u;
			bestMove = &possible[i];
		}
	}
	
	Q_ASSERT(bestMove != 0);
	return *bestMove;
}

MC_T
MeepleMove MonteCarloPlayer MC_TU::getMeepleMove(int player, const Tile * /*tile*/, const MoveHistoryEntry & move, const MeepleMovesType & possible)
{
	QElapsedTimer timer;
	if (useTimeout)
		timer.start();

	int const playerCount = game->getPlayerCount();
	int const possibleSize = possible.size();
	auto utilities = QVarLengthArray<RewardType, 128>(possibleSize);
	for (int i = 0; i < possibleSize; ++i)
		utilities[i] = 0;
	
//	Q_ASSERT(game->equals(g));	//Does not equal, since game as the tile already placed, while g doesn't
	
	MoveHistoryEntry m = move;
	int i = 0;
	do
	{
		int moveIndex = 0;
		for (MeepleMove const & meepleMove : possible)
		{
			m.move.meepleMove = meepleMove;
#if USE_RESET
			g.restartGame(history);
			if (g.step(m))
				while (g.simStep(&RandomPlayer::instance))
					;

			utilities[moveIndex] += utility(g.getScores(), playerCount, player);
#else
			int steps = 1;
			if (simGame->simStep(m))
			{
				do
					++steps;
				while (simGame->simStep(&RandomPlayer::instance));
			}
			
			utilities[moveIndex] += utility(simGame->getScores(), playerCount, player, simGame);
			
			for (int i = 0; i < steps; ++i)
				simGame->simUndo();
//			Q_ASSERT(game->equals(g));
#endif
			++moveIndex;
		}
		if (!useTimeout)
			i += possibleSize;
#if COUNT_PLAYOUTS
		else
			playouts += possibleSize;
#endif
	} while (useTimeout ? !timer.hasExpired((1*M)/3) : i < M);

#if COUNT_PLAYOUTS
	if (!useTimeout)
		playouts += i;
#endif
	
	RewardType bestUtility = std::numeric_limits<RewardType>::has_infinity ? (-std::numeric_limits<RewardType>::infinity()) : std::numeric_limits<RewardType>::lowest();
	MeepleMove const * bestMove = 0;
	for (int i = 0; i < possibleSize; ++i)
	{
		auto u = utilities[i];
		if (u > bestUtility)
		{
			bestUtility = u;
			bestMove = &possible[i];
		}
	}
	
	Q_ASSERT(bestMove != 0);
	return *bestMove;
}

MC_T
void MonteCarloPlayer MC_TU::endGame()
{
	delete simGame;
	simGame = 0;
}

MC_T
Player * MonteCarloPlayer MC_TU::clone() const
{
	return new MonteCarloPlayer(tileFactory, M, useTimeout);
}
