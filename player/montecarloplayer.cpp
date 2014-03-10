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

TileMove MonteCarloPlayer::getTileMove(int player, const Tile * const /*tile*/, const MoveHistoryEntry & move, const TileMovesType & placements, const Game * const game)
{
	int const playerCount = game->getPlayerCount();
	auto utilities = QVarLengthArray<long long int, 128>(placements.size());

	for (int j = 0; j < 1000; ++j)
	{
		int moveIndex = 0;
		for (TileMove const & tileMove : placements)
		{
			Game g;
			for (int i = 0; i < playerCount; ++i)
				g.addPlayer(&RandomPlayer::instance);
			g.newGame(game->getTileSets(), tileFactory, game->getMoveHistory());
	//		g.step(move.tile, tileMove, player, this);
			g.step(move.tile, tileMove, player, &RandomPlayer::instance);
			
			while (!g.isFinished())
				g.step();
			utilities[moveIndex] += utility(g.getScores(), g.getPlayerCount(), player);
			++moveIndex;
		}
	}
	
	long long int bestUtility = std::numeric_limits<long long int>::min();
	TileMove const * bestMove = 0;
	for (int i = 0; i < playerCount; ++i)
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
	auto utilities = QVarLengthArray<long long int, 128>(possible.size());
	
	for (int j = 0; j < 1000; ++j)
	{
		int moveIndex = 0;
		for (MeepleMove const & meepleMove : possible)
		{
			MoveHistoryEntry m = move;
			m.move.meepleMove = meepleMove;
			Game g;
			for (int i = 0; i < playerCount; ++i)
				g.addPlayer(&RandomPlayer::instance);
			g.newGame(game->getTileSets(), tileFactory, game->getMoveHistory());
			g.step(m);
			
			while (!g.isFinished())
				g.step();
			utilities[moveIndex] += utility(g.getScores(), g.getPlayerCount(), player);
			++moveIndex;
		}
	}
	
	long long int bestUtility = std::numeric_limits<long long int>::min();
	MeepleMove const * bestMove = 0;
	for (int i = 0; i < playerCount; ++i)
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
