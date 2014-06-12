#include "montecarloplayeruct.h"
#include "randomplayer.h"
#include "core/util.h"

#define MCU_T template<class UtilityProvider, class Playout>
#define MCU_TU <UtilityProvider, Playout>

MCU_T
Util::Math const & MonteCarloPlayerUCT MCU_TU::math = Util::Math::instance;

MCU_T
void MonteCarloPlayerUCT MCU_TU::newGame(int player, const Game * g)
{
	uint const playerCount = g->getPlayerCount();
	if (simGame == 0)
		simGame = new Game(0);
	game = g;
	simGame->clearPlayers();
	for (uint i = 0; i < playerCount; ++i)
		simGame->addPlayer(&RandomPlayer::instance);
	simGame->newGame(g->getTileSets(), tileFactory, g->getMoveHistory());
	utilityProvider.newGame(player, g);
	playoutPolicy.newGame(player, g);
}

MCU_T
void MonteCarloPlayerUCT MCU_TU::playerMoved(int /*player*/, const Tile * /*tile*/, const MoveHistoryEntry & /*move*/)
{
	Util::syncGamesFast(*game, *simGame);
}

MCU_T
void MonteCarloPlayerUCT MCU_TU::undoneMove(const MoveHistoryEntry & /*move*/)
{
	Util::syncGames(*game, *simGame);
}

MCU_T
TileMove MonteCarloPlayerUCT MCU_TU::getTileMove(int player, const Tile * /*tile*/, const MoveHistoryEntry & move, const TileMovesType & possible)
{
	Util::ExpireTimer timer;
	if (useTimeout)
		timer.start();

	Util::syncGamesFast(*game, *simGame);

	Q_ASSERT(game->equals(*simGame));

	uint const playerCount = game->getPlayerCount();
	int const possibleSize = possible.size();
	auto meepleMoves = VarLengthArrayWrapper<MeepleMovesType, 128>::type(possibleSize);
//	int rewards0 = 0;
	uint playoutCount0 = 0;
	auto rewards1 = typename VarLengthArrayWrapper<RewardType, 128>::type(possibleSize);
	auto playoutCount1 = VarLengthArrayWrapper<uint, 128>::type(possibleSize);
	auto rewards2 = typename VarLengthArrayWrapper<typename VarLengthArrayWrapper<RewardType, 16>::type, 128>::type(possibleSize);
	auto playoutCount2 = VarLengthArrayWrapper<VarLengthArrayWrapper<uint, 16>::type, 128>::type(possibleSize);
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
			rewards1[i] = 0;
			playoutCount1[i] = 0;
			rewards2[i] = typename VarLengthArrayWrapper<RewardType, 16>::type(mms);
			playoutCount2[i] = VarLengthArrayWrapper<uint, 16>::type(mms);

			for (int j = 0; j < mms; ++j)
			{
				simGame->simPartStepMeeple(mm[j]);

				int steps = playout();
				RewardType const u = utility(simGame->getScores(), playerCount, player, simGame);
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
	int i = 0;
	do
	{
		int const tmIndex = chooseTileMove(possible, playoutCount0, rewards1, playoutCount1);
		int const mmIndex = chooseMeepleMove(meepleMoves[tmIndex], playoutCount1[tmIndex], rewards2[tmIndex], playoutCount2[tmIndex]);
		simMove.move.tileMove = possible[tmIndex];
		simMove.move.meepleMove = meepleMoves[tmIndex][mmIndex];
		simGame->simStep(simMove);

		int steps = playout();
		RewardType const u = utility(simGame->getScores(), playerCount, player, simGame);
		rewards1[tmIndex] += u;
		rewards2[tmIndex][mmIndex] += u;
		++playoutCount0;
		++playoutCount1[tmIndex];
		++playoutCount2[tmIndex][mmIndex];
		unplayout(steps+1);
		Q_ASSERT(game->equals(*simGame));
	} while (useTimeout ? !timer.hasExpired(M) : i < M);
#if COUNT_PLAYOUTS
	playouts += playoutCount0;
#endif

	// choose tile move
	int bestMoveIndex = -1;
	qreal bestUtility = -std::numeric_limits<qreal>::infinity();
	for (int i = 0; i < possibleSize; ++i)
	{
		qreal u = qreal(rewards1[i]) / qreal(playoutCount1[i]);
		if (u > bestUtility)
		{
			bestUtility = u;
			bestMoveIndex = i;
		}
	}

	// choose meeple move
	bestUtility = -std::numeric_limits<qreal>::infinity();
	auto const & rew = rewards2[bestMoveIndex];
	for (int j = 0, s = rew.size(); j < s; ++j)
	{
		auto u = qreal(rew[j]) / qreal(playoutCount2[bestMoveIndex][j]);
		if (u > bestUtility)
		{
			bestUtility = u;
			meepleMove = meepleMoves[bestMoveIndex][j];
		}
	}

	Q_ASSERT(bestMoveIndex != -1);
	return possible[bestMoveIndex];
}

MCU_T
MeepleMove MonteCarloPlayerUCT MCU_TU::getMeepleMove(int /*player*/, const Tile * /*tile*/, const MoveHistoryEntry & /*move*/, const MeepleMovesType & /*possible*/)
{
	return meepleMove;
}

MCU_T
void MonteCarloPlayerUCT MCU_TU::endGame()
{
	delete simGame;
	simGame = 0;
}

MCU_T
Player * MonteCarloPlayerUCT MCU_TU::clone() const
{
	return new MonteCarloPlayerUCT(tileFactory, M, useTimeout);
}

MCU_T
int MonteCarloPlayerUCT MCU_TU::playout()
{
	return playoutPolicy.playout(*simGame);
}

MCU_T
void MonteCarloPlayerUCT MCU_TU::unplayout(int steps)
{
	playoutPolicy.undoPlayout(*simGame, steps);
}
