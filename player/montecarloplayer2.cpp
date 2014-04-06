#include "montecarloplayer2.h"
#include "randomplayer.h"
#include <QElapsedTimer>

RandomTable MonteCarloPlayer2::r = RandomTable();

void MonteCarloPlayer2::newGame(int /*player*/, const Game * g)
{
	if (simGame == 0)
		simGame = new Game(0);
	game = g;
	simGame->clearPlayers();
	for (uint i = 0; i < g->getPlayerCount(); ++i)
		simGame->addPlayer(&RandomPlayer::instance);
	simGame->newGame(game->getTileSets(), tileFactory, g->getMoveHistory());
}

void MonteCarloPlayer2::playerMoved(int /*player*/, const Tile * /*tile*/, const MoveHistoryEntry & /*move*/)
{
	auto const & history = game->getMoveHistory();
	for (uint i = simGame->getMoveHistory().size(); i < history.size(); ++i)
		simGame->simStep(history[i]);
}

TileMove MonteCarloPlayer2::getTileMove(int player, const Tile * /*tile*/, const MoveHistoryEntry & move, const TileMovesType & possible)
{
#ifdef TIMEOUT
	QElapsedTimer timer;
	timer.start();
#endif

	Util::syncGamesFast(*game, *simGame);

	Q_ASSERT(game->equals(*simGame));

	int const playerCount = game->getPlayerCount();
	int const possibleSize = possible.size();
	auto meepleMoves = VarLengthArrayWrapper<MeepleMovesType, 128>::type(possibleSize);
	auto rewards = VarLengthArrayWrapper<VarLengthArrayWrapper<int, 16>::type, 128>::type(possibleSize);
	auto playoutCount = VarLengthArrayWrapper<VarLengthArrayWrapper<int, 16>::type, 128>::type(possibleSize);
	{
		Tile * simTile = simGame->getTiles()[move.tile];
		simGame->simPartStepChance(move.tile);
		for (int i = 0; i < possibleSize; ++i)
		{
			TileMove const & tm = possible[i];
			simGame->simPartStepTile(tm);

			MeepleMovesType mm;
			if (simGame->getPlayerMeeples(player) > 0)
				mm = simGame->getPossibleMeeplePlacements(simTile);
			else
				mm.push_back(MeepleMove());
			int const mms = mm.size();
			meepleMoves[i] = mm;
			rewards[i] = VarLengthArrayWrapper<int, 16>::type(mms);
			playoutCount[i] = VarLengthArrayWrapper<int, 16>::type(mms);

			for (int j = 0; j < mms; ++j)
			{
				simGame->simPartStepMeeple(mm[j]);

				int steps = playout();
				rewards[i][j] = utility(simGame->getScores(), playerCount, player);
				playoutCount[i][j] = 1;
				unplayout(steps);

				simGame->simPartUndoMeeple();
			}

			simGame->simPartUndoTile();
		}
		simGame->simPartUndoChance();
	}

	Q_ASSERT(game->equals(*simGame));

	MoveHistoryEntry simMove = move;
#ifdef TIMEOUT
	while (!timer.hasExpired(TIMEOUT))
#else
	for (int j = 0; j < N; ++j)
#endif
	{
		int const tmIndex = chooseTileMove(possible);
		int const mmIndex = chooseMeepleMove(meepleMoves[tmIndex]);
		simMove.move.tileMove = possible[tmIndex];
		simMove.move.meepleMove = meepleMoves[tmIndex][mmIndex];
		simGame->simStep(simMove);

		int steps = playout();
		rewards[tmIndex][mmIndex] += utility(simGame->getScores(), playerCount, player);
		++playoutCount[tmIndex][mmIndex];
		unplayout(steps+1);
		Q_ASSERT(game->equals(*simGame));

#if COUNT_PLAYOUTS
		++playouts;
#endif
	}

	TileMove const * bestMove = 0;
	qreal bestUtility = -std::numeric_limits<qreal>::infinity();
	for (int i = 0; i < possibleSize; ++i)
	{
		VarLengthArrayWrapper<int, 16>::type const & rew = rewards[i];
		for (int j = 0; j < rew.size(); ++j)
		{
			auto u = rew[j] / qreal(playoutCount[i][j]);
			if (u > bestUtility)
			{
//				qDebug() << i << rew[j] << "/" << playoutCount[i][j] << "=" << u;
				bestUtility = u;
				bestMove = &possible[i];
				meepleMove = meepleMoves[i][j];
			}
		}
	}

	Q_ASSERT(bestMove != 0);
	return *bestMove;
}

MeepleMove MonteCarloPlayer2::getMeepleMove(int /*player*/, const Tile * /*tile*/, const MoveHistoryEntry & /*move*/, const MeepleMovesType & /*possible*/)
{
	return meepleMove;
}

void MonteCarloPlayer2::endGame()
{
	delete simGame;
	simGame = 0;
}

int MonteCarloPlayer2::playout()
{
	int steps = 0;
	if (!simGame->isFinished())
	{
		do
		{
			++steps;
		} while (simGame->simStep(&RandomPlayer::instance));
	}
	return steps;
}

void MonteCarloPlayer2::unplayout(int steps)
{
	for (; steps > 0; --steps)
		simGame->undo();
}
