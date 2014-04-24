#include "montecarloplayer.h"
#include "randomplayer.h"
#include <QElapsedTimer>

#define MC_T template<class UtilityProvider, class Playout>
#define MC_TU <UtilityProvider, Playout>

MC_T
void MonteCarloPlayer MC_TU::newGame(int /*player*/, const Game * g)
{
	if (simGame == 0)
		simGame = new Game(0);
	game = g;
	simGame->clearPlayers();
	for (uint i = 0; i < g->getPlayerCount(); ++i)
		simGame->addPlayer(&RandomPlayer::instance);
	simGame->newGame(game->getTileSets(), tileFactory, g->getMoveHistory());
}

MC_T
void MonteCarloPlayer MC_TU::playerMoved(int /*player*/, const Tile * /*tile*/, const MoveHistoryEntry & /*move*/)
{
	Util::syncGamesFast(*game, *simGame);
}

MC_T
TileMove MonteCarloPlayer MC_TU::getTileMove(int player, const Tile * /*tile*/, const MoveHistoryEntry & move, const TileMovesType & possible)
{
#ifdef TIMEOUT
	QElapsedTimer timer;
	timer.start();
#endif

	Util::syncGamesFast(*game, *simGame);

	int const playerCount = game->getPlayerCount();
	int const possibleSize = possible.size();
	auto utilities = QVarLengthArray<long long int, 128>(possibleSize);
	for (int i = 0; i < possibleSize; ++i)
		utilities[i] = 0;

	Q_ASSERT(game->equals(*simGame));
	
#ifdef TIMEOUT
	while (!timer.hasExpired((2*TIMEOUT)/3))
#else
	for (int j = 0; j < N; ++j)
#endif
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
			simGame->simPartStepChance(move.tile);
			simGame->simPartStepTile(tileMove);
			MeepleMove meepleMove;
			if (simGame->getPlayerMeeples(player) > 0)
				meepleMove = playoutPolicy.chooseMeepleMove(player, simGame->simTile, simGame->simEntry, simGame->getPossibleMeeplePlacements(simGame->simTile));
			simGame->simPartStepMeeple(meepleMove);
			int steps = 1 + playoutPolicy.playout(*simGame);
			
			utilities[moveIndex] += utility(simGame->getScores(), playerCount, player);
			
			playoutPolicy.undoPlayout(*simGame, steps);
			Q_ASSERT(game->equals(*simGame));
#endif
			++moveIndex;
		}
#if defined(TIMEOUT) && COUNT_PLAYOUTS
		playouts += possibleSize;
#endif
	}
#if !defined(TIMEOUT) && COUNT_PLAYOUTS
	playouts += N * possibleSize;
#endif
	
	TileMove const * bestMove = 0;
	long long int bestUtility = std::numeric_limits<long long int>::min();
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
#ifdef TIMEOUT
	QElapsedTimer timer;
	timer.start();
#endif

	int const playerCount = game->getPlayerCount();
	int const possibleSize = possible.size();
	auto utilities = QVarLengthArray<long long int, 128>(possibleSize);
	for (int i = 0; i < possibleSize; ++i)
		utilities[i] = 0;
	
//	Q_ASSERT(game->equals(g));	//Does not equal, since game as the tile already placed, while g doesn't
	
	MoveHistoryEntry m = move;
#ifdef TIMEOUT
	while (!timer.hasExpired((1*TIMEOUT)/3))
#else
	for (int j = 0; j < N; ++j)
#endif
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
			
			utilities[moveIndex] += utility(simGame->getScores(), playerCount, player);
			
			for (int i = 0; i < steps; ++i)
				simGame->undo();
//			Q_ASSERT(game->equals(g));
#endif
			++moveIndex;
		}
#if defined(TIMEOUT) && COUNT_PLAYOUTS
		playouts += possibleSize;
#endif
	}
#if !defined(TIMEOUT) && COUNT_PLAYOUTS
	playouts += N * possibleSize;
#endif
	
	long long int bestUtility = std::numeric_limits<long long int>::min();
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
