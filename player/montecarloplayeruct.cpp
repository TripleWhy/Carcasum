#include "montecarloplayeruct.h"
#include "randomplayer.h"
#include <QElapsedTimer>

RandomTable MonteCarloPlayerUCT::r = RandomTable();

void MonteCarloPlayerUCT::newGame(int /*player*/, const Game * g)
{
	if (simGame == 0)
		simGame = new Game(0);
	game = g;
	simGame->clearPlayers();
	for (uint i = 0; i < g->getPlayerCount(); ++i)
		simGame->addPlayer(&RandomPlayer::instance);
	simGame->newGame(game->getTileSets(), tileFactory, g->getMoveHistory());
}

void MonteCarloPlayerUCT::playerMoved(int /*player*/, const Tile * /*tile*/, const MoveHistoryEntry & /*move*/)
{
	auto const & history = game->getMoveHistory();
	for (uint i = simGame->getMoveHistory().size(); i < history.size(); ++i)
		simGame->simStep(history[i]);
}

TileMove MonteCarloPlayerUCT::getTileMove(int player, const Tile * /*tile*/, const MoveHistoryEntry & move, const TileMovesType & possible)
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
//	int rewards0 = 0;
	int playoutCount0 = 0;
	auto rewards1 = VarLengthArrayWrapper<int, 128>::type(possibleSize);
	auto playoutCount1 = VarLengthArrayWrapper<int, 128>::type(possibleSize);
	auto rewards2 = VarLengthArrayWrapper<VarLengthArrayWrapper<int, 16>::type, 128>::type(possibleSize);
	auto playoutCount2 = VarLengthArrayWrapper<VarLengthArrayWrapper<int, 16>::type, 128>::type(possibleSize);
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
			rewards1[i] = 0;
			playoutCount1[i] = 0;
			rewards2[i] = VarLengthArrayWrapper<int, 16>::type(mms);
			playoutCount2[i] = VarLengthArrayWrapper<int, 16>::type(mms);

			for (int j = 0; j < mms; ++j)
			{
				simGame->simPartStepMeeple(mm[j]);

				int steps = playout();
				int const u = utility(simGame->getScores(), playerCount, player);
//				rewards0 += u;
				++playoutCount0;
				rewards1[i] += u;
				++playoutCount1[i];
				rewards2[i][j] = u;
				playoutCount2[i][j] = 1;
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
		int const tmIndex = chooseTileMove(possible, playoutCount0, rewards1, playoutCount1);
		int const mmIndex = chooseMeepleMove(meepleMoves[tmIndex], playoutCount1[tmIndex], rewards2[tmIndex], playoutCount2[tmIndex]);
		simMove.move.tileMove = possible[tmIndex];
		simMove.move.meepleMove = meepleMoves[tmIndex][mmIndex];
		simGame->simStep(simMove);

		int steps = playout();
		int const u = utility(simGame->getScores(), playerCount, player);
		rewards1[tmIndex] += u;
		rewards2[tmIndex][mmIndex] += u;
		++playoutCount0;
		++playoutCount1[tmIndex];
		++playoutCount2[tmIndex][mmIndex];
		unplayout(steps+1);
		Q_ASSERT(game->equals(*simGame));
	}
#if COUNT_PLAYOUTS
	playouts += playoutCount0;
#endif

	// choose tile move
	int bestMoveIndex = -1;
	qreal bestUtility = -std::numeric_limits<qreal>::infinity();
	for (int i = 0; i < possibleSize; ++i)
	{
		auto u = rewards1[i] / qreal(playoutCount1[i]);
		if (u > bestUtility)
		{
			bestUtility = u;
			bestMoveIndex = i;
		}
	}

	// choose meeple move
	bestUtility = -std::numeric_limits<qreal>::infinity();
	VarLengthArrayWrapper<int, 16>::type const & rew = rewards2[bestMoveIndex];
	for (int j = 0; j < rew.size(); ++j)
	{
		auto u = rew[j] / qreal(playoutCount2[bestMoveIndex][j]);
		if (u > bestUtility)
		{
			bestUtility = u;
			meepleMove = meepleMoves[bestMoveIndex][j];
		}
	}

	Q_ASSERT(bestMoveIndex != -1);
	return possible[bestMoveIndex];
}

MeepleMove MonteCarloPlayerUCT::getMeepleMove(int /*player*/, const Tile * /*tile*/, const MoveHistoryEntry & /*move*/, const MeepleMovesType & /*possible*/)
{
	return meepleMove;
}

void MonteCarloPlayerUCT::endGame()
{
	delete simGame;
	simGame = 0;
}

int MonteCarloPlayerUCT::playout()
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

void MonteCarloPlayerUCT::unplayout(int steps)
{
	for (; steps > 0; --steps)
		simGame->undo();
}
