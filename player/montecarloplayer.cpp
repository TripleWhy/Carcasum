#include "montecarloplayer.h"

#include "randomplayer.h"

MonteCarloPlayer::MonteCarloPlayer(jcz::TileFactory * tileFactory)
    : tileFactory(tileFactory)
{
}

MonteCarloPlayer::~MonteCarloPlayer()
{
	
}

void MonteCarloPlayer::newGame(int player, const Game * const game)
{
	Q_UNUSED(player);
	Q_UNUSED(game);
}

void MonteCarloPlayer::playerMoved(int player, const Tile * const tile, const MoveHistoryEntry & move, const Game * const game)
{
	Q_UNUSED(player);
	Q_UNUSED(tile);
	Q_UNUSED(move);
	Q_UNUSED(game);
}

int const threadCount = 8;
TileMove MonteCarloPlayer::getTileMove(int player, const Tile * const /*tile*/, const MoveHistoryEntry & move, const TileMovesType & placements, const Game * const game)
{
	int const playerCount = game->getPlayerCount();
	int const placementSize = placements.size();
	auto const & history = game->getMoveHistory();
	auto const & tileSets = game->getTileSets();

	std::thread * threads[threadCount];
	int threadPlayouts[threadCount];
	QVarLengthArray<long long int, 128> threadUtilities[threadCount];

	for (int i = 0; i < threadCount; ++i)
	{
		threads[i] = new std::thread( [&threadPlayouts, &threadUtilities, i, playerCount, placementSize, &history, &tileSets, this, &placements, &move, player, game]()
		{
			threadPlayouts[i] = 0;
			threadUtilities[i] = QVarLengthArray<long long int, 128>(placementSize);
			for (int j = 0; j < placementSize; ++j)
				threadUtilities[i][j] = 0;

			Game g;
			for (int i = 0; i < playerCount; ++i)
				g.addPlayer(&RandomPlayer::instance);
			g.newGame(tileSets, tileFactory, history);
//			Q_ASSERT(game->equals(g));

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
						g.step();

					threadUtilities[i][moveIndex] += utility(g.getScores(), playerCount, player);
#else
					g.step(move.tile, tileMove, player, &RandomPlayer::instance);

					int steps = 1;
					for ( ; !g.isFinished(); ++steps)
						g.step();

					threadUtilities[i][moveIndex] += utility(g.getScores(), playerCount, player);

					for (int i = 0; i < steps; ++i)
						g.undo();
//					Q_ASSERT(game->equals(g));
#endif
				}
#if COUNT_PLAYOUTS
				threadPlayouts[i] += N;
#endif
				++moveIndex;
			}
		} );
	}

	for (int i = 0; i < threadCount; ++i)
	{
		threads[i]->join();
		playouts += threadPlayouts[i];
	}

	auto & utilities = threadUtilities[0];
	for (int i = 1; i < threadCount; ++i)
	{
		for (int j = 0; j < placementSize; ++j)
			utilities[j] += threadUtilities[i][j];
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

MeepleMove MonteCarloPlayer::getMeepleMove(int player, const Tile * const /*tile*/, const MoveHistoryEntry & move, const MeepleMovesType & possible, const Game * const game)
{
	int const playerCount = game->getPlayerCount();
	int const possibleSize = possible.size();
	auto const & history = game->getMoveHistory();
	auto const & tileSets = game->getTileSets();

	std::thread * threads[threadCount];
	int threadPlayouts[threadCount];
	QVarLengthArray<long long int, 128> threadUtilities[threadCount];

	for (int i = 0; i < threadCount; ++i)
	{
		threads[i] = new std::thread( [&threadPlayouts, &threadUtilities, i, playerCount, possibleSize, &history, &tileSets, this, &possible, &move, player, game]()
		{
			threadPlayouts[i] = 0;
			threadUtilities[i] = QVarLengthArray<long long int, 128>(possibleSize);
			for (int j = 0; j < possibleSize; ++j)
				threadUtilities[i][j] = 0;

			Game g;
			for (int i = 0; i < playerCount; ++i)
				g.addPlayer(&RandomPlayer::instance);
			g.newGame(tileSets, tileFactory, history);
//			Q_ASSERT(game->equals(g));

			int moveIndex = 0;
			for (MeepleMove const & meepleMove : possible)
			{
				MoveHistoryEntry m = move;
				m.move.meepleMove = meepleMove;
				for (int j = 0; j < N; ++j)
				{
#if USE_RESET
			g.restartGame(history);
			g.step(m);

			while (!g.isFinished())
				g.step();

			threadUtilities[i][moveIndex] += utility(g.getScores(), playerCount, player);
#else
			g.step(m);

			int steps = 1;
			for ( ; !g.isFinished(); ++steps)
				g.step();

			threadUtilities[i][moveIndex] += utility(g.getScores(), playerCount, player);

			for (int i = 0; i < steps; ++i)
				g.undo();
//			Q_ASSERT(game->equals(g));
#endif
				}
#if COUNT_PLAYOUTS
				threadPlayouts[i] += N;
#endif
				++moveIndex;
			}
		} );
	}

	for (int i = 0; i < threadCount; ++i)
	{
		threads[i]->join();
		playouts += threadPlayouts[i];
	}

	auto & utilities = threadUtilities[0];
	for (int i = 1; i < threadCount; ++i)
	{
		for (int j = 0; j < possibleSize; ++j)
			utilities[j] += threadUtilities[i][j];
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
