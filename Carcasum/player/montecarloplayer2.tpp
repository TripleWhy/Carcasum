#include "montecarloplayer2.h"
#include "randomplayer.h"
#include <QElapsedTimer>

#define MC2_T template<class UtilityProvider, class Playout>
#define MC2_TU <UtilityProvider, Playout>

MC2_T
RandomTable MonteCarloPlayer2 MC2_TU::r = RandomTable();

MC2_T
void MonteCarloPlayer2 MC2_TU::newGame(int /*player*/, const Game * g)
{
	if (simGame == 0)
		simGame = new Game(0);
	game = g;
	simGame->clearPlayers();
	for (uint i = 0; i < g->getPlayerCount(); ++i)
		simGame->addPlayer(&RandomPlayer::instance);
	simGame->newGame(game->getTileSets(), tileFactory, g->getMoveHistory());
}

MC2_T
void MonteCarloPlayer2 MC2_TU::playerMoved(int /*player*/, const Tile * /*tile*/, const MoveHistoryEntry & /*move*/)
{
	Util::syncGamesFast(*game, *simGame);
}

MC2_T
TileMove MonteCarloPlayer2 MC2_TU::getTileMove(int player, const Tile * /*tile*/, const MoveHistoryEntry & move, const TileMovesType & possible)
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
	auto rewards = QVarLengthArray<QVarLengthArray<RewardType, 16>, 128>(possibleSize);	//VarLengthArrayWrapper does not work?!
	auto playoutCount = VarLengthArrayWrapper<VarLengthArrayWrapper<int, 16>::type, 128>::type(possibleSize);
	{
		simGame->simPartStepChance(move.tileIndex);
		Tile * simTile = simGame->simTile;
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
			rewards[i] = QVarLengthArray<RewardType, 16>(mms);
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
	}

	TileMove const * bestMove = 0;
	qreal bestUtility = -std::numeric_limits<qreal>::infinity();
	for (int i = 0; i < possibleSize; ++i)
	{
		auto const & rew = rewards[i];
		for (int j = 0; j < rew.size(); ++j)
		{
#if COUNT_PLAYOUTS
			playouts += playoutCount[i][j];
#endif
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

MC2_T
MeepleMove MonteCarloPlayer2 MC2_TU::getMeepleMove(int /*player*/, const Tile * /*tile*/, const MoveHistoryEntry & /*move*/, const MeepleMovesType & /*possible*/)
{
	return meepleMove;
}

MC2_T
void MonteCarloPlayer2 MC2_TU::endGame()
{
	delete simGame;
	simGame = 0;
}

MC2_T
int MonteCarloPlayer2 MC2_TU::playout()
{
	return playoutPolicy.playout(*simGame);
}

MC2_T
void MonteCarloPlayer2 MC2_TU::unplayout(int steps)
{
	playoutPolicy.undoPlayout(*simGame, steps);
}
