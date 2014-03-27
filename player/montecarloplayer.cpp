#include "montecarloplayer.h"
#include "randomplayer.h"

MonteCarloPlayer::MonteCarloPlayer(jcz::TileFactory * tileFactory)
    : tileFactory(tileFactory)
{
}

MonteCarloPlayer::~MonteCarloPlayer()
{
	
}

void MonteCarloPlayer::newGame(int /*player*/, const Game * g)
{
	game = g;
	simGame.clearPlayers();
	for (uint i = 0; i < g->getPlayerCount(); ++i)
		simGame.addPlayer(&RandomPlayer::instance);
	simGame.newGame(game->getTileSets(), tileFactory, g->getMoveHistory());
}

void MonteCarloPlayer::playerMoved(int /*player*/, const Tile * /*tile*/, const MoveHistoryEntry & /*move*/)
{
	auto const & history = game->getMoveHistory();
	for (uint i = simGame.getMoveHistory().size(); i < history.size(); ++i)
		simGame.simStep(history[i]);
}

TileMove MonteCarloPlayer::getTileMove(int player, const Tile * /*tile*/, const MoveHistoryEntry & move, const TileMovesType & placements)
{
	int const playerCount = game->getPlayerCount();
	int const placementSize = placements.size();
	auto utilities = QVarLengthArray<long long int, 128>(placementSize);
	for (int i = 0; i < placementSize; ++i)
		utilities[i] = 0;

	Q_ASSERT(game->equals(simGame));
	
	int moveIndex = 0;
	for (TileMove const & tileMove : placements)
	{
		for (int j = 0; j < N; ++j)
		{
#if USE_RESET
			g.restartGame(history);
			Q_ASSERT(game->equals(g));
			g.step(move.tile, tileMove, player, &RandomPlayer::instance);

			while (!g.isFinished())
				g.simStep(&RandomPlayer::instance);

			utilities[moveIndex] += utility(g.getScores(), playerCount, player);
#else
			simGame.simStep(move.tile, tileMove, player, &RandomPlayer::instance);

			int steps = 1;
			for ( ; !simGame.isFinished(); ++steps)
				simGame.simStep(&RandomPlayer::instance);
			
			utilities[moveIndex] += utility(simGame.getScores(), playerCount, player);
			
			for (int i = 0; i < steps; ++i)
				simGame.undo();
			Q_ASSERT(game->equals(simGame));
#endif
		}
#if COUNT_PLAYOUTS
			playouts += N;
#endif
		++moveIndex;
	}
	
	long long int bestUtility = std::numeric_limits<long long int>::min();
	TileMove const * bestMove = 0;
	for (int i = 0; i < placementSize; ++i)
	{
		auto u = utilities[i];
		if (u > bestUtility)
		{
			bestUtility = u;
			bestMove = &placements[i];
		}
	}
	
	Q_ASSERT(bestMove != 0);
	return *bestMove;
}

MeepleMove MonteCarloPlayer::getMeepleMove(int player, const Tile * /*tile*/, const MoveHistoryEntry & move, const MeepleMovesType & possible)
{
	int const playerCount = game->getPlayerCount();
	int const possibleSize = possible.size();
	auto utilities = QVarLengthArray<long long int, 128>(possibleSize);
	for (int i = 0; i < possibleSize; ++i)
		utilities[i] = 0;
	
//	Q_ASSERT(game->equals(g));	//Does not equal, since game as the tile already placed, while g doesn't
	
	int moveIndex = 0;
	for (MeepleMove const & meepleMove : possible)
	{
		MoveHistoryEntry m = move;
		m.move.meepleMove = meepleMove;
		for (int j = 0; j < N; ++j)
		{
#if USE_RESET
			g.restartGame(history);
			if (g.step(m))
				while (g.simStep(&RandomPlayer::instance))
					;

			utilities[moveIndex] += utility(g.getScores(), playerCount, player);
#else
			int steps = 1;
			if (simGame.simStep(m))
			{
				do
					++steps;
				while (simGame.simStep(&RandomPlayer::instance));
			}
			
			utilities[moveIndex] += utility(simGame.getScores(), playerCount, player);
			
			for (int i = 0; i < steps; ++i)
				simGame.undo();
//			Q_ASSERT(game->equals(g));
#endif
		}
#if COUNT_PLAYOUTS
			playouts += N;
#endif
		++moveIndex;
	}
	
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

void MonteCarloPlayer::endGame()
{
}
