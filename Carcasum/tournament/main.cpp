#include "static.h"
#include "core/game.h"
#include "player/randomplayer.h"
#include "player/montecarloplayer.h"
#include "player/montecarloplayer2.h"
#include "player/montecarloplayeruct.h"
#include "player/mctsplayer.h"
#include <QCoreApplication>
#include <QElapsedTimer>
#include <QVarLengthArray>

struct Result
{
	QVarLengthArray<int, MAX_PLAYERS> scores;
	QVarLengthArray<int, MAX_PLAYERS> utilities;
	QVarLengthArray<quint64, MAX_PLAYERS> playouts;

	Result(int size) : scores(size), utilities(size), playouts(size) {}
};

void printResults(std::vector<Result> const & results, int const playerCount)
{
	QVarLengthArray<int, MAX_PLAYERS> wins(playerCount);
	QVarLengthArray<int, MAX_PLAYERS> draws(playerCount);
	QVarLengthArray<int, MAX_PLAYERS> scores(playerCount);
	QVarLengthArray<quint64, MAX_PLAYERS> playouts(playerCount);
	for (int i = 0; i < playerCount; ++i)
	{
		wins[i] = 0;
		draws[i] = 0;
		scores[i] = 0;
		playouts[i] = 0;
	}

	for (Result const & r : results)
	{
		for (int i = 0; i < playerCount; ++i)
		{
			if (r.utilities[i] == 1)
				++wins[i];
			else if (r.utilities[i] == 0)
				++draws[i];
			scores[i] += r.scores[i];
			playouts[i] += r.playouts[i];
		}
	}

	for (int i = 0; i < playerCount; ++i)
	{
		qDebug() << "player" << i << "   wins:" << wins[i] << "/" << results.size() << "=" << (wins[i] / qreal(results.size()))
		         <<"   draws:" << draws[i] << "/" << results.size() << "=" << (draws[i] / qreal(results.size()))
		         << "   avg. score:" << (scores[i] / qreal(results.size()))
		         << "\tavg. playouts:" << (qreal(playouts[i]) / qreal(results.size()));
	}
}

void printTimes(QVarLengthArray<quint64, MAX_PLAYERS> const & times, QVarLengthArray<qint64, MAX_PLAYERS> const & diffs, QVarLengthArray<int, MAX_PLAYERS> const & steps, std::vector<Player *> const & players)
{
	for (int i = 0; i < times.size(); ++i)
	{
		qreal s = qreal(quint64(steps[i]) * quint64(1000000));
		Player * p = players[i];
		qDebug() << "player" << i << "   playouts:" << p->playouts << "\tavg. thinking time:" << ((qreal)times[i] / s) << "ms   avg. overrun:" << ((qreal)diffs[i] / s) << "ms";
	}
}

int main(int argc, char *argv[])
{
	if (false)
	{
		QElapsedTimer t;
		auto const & math = Util::Math::instance;

		for (uint i = 0; i < LN_TABLE_SIZE; ++i)
		{
			qreal l1 = ::log(i);
			qreal l2 = math.ln(i);
			if (l1 != l2)
			{
				qDebug() << "i: " << i << l1 << l2;
			}
		}

		for (int i = 0; i < 3; ++i)
		{
			qreal sum = 0;
			t.start();
			for (int k = 0; k < 1000; ++k)
			{
				for (uint j = 1; j < 1000000; ++j)
					sum += ::log(j);
			}
			auto e = t.elapsed();
			qDebug() << e << sum;
		}
		qDebug();

		for (int i = 0; i < 3; ++i)
		{
			qreal sum = 0;
			t.start();
			for (int k = 0; k < 1000; ++k)
			{
				for (uint j = 1; j < 1000000; ++j)
					sum += math.ln(j);
			}
			auto e = t.elapsed();
			qDebug() << e << sum;
		}

		return 0;
	}

	QCoreApplication app(argc, argv);
	jcz::TileFactory * tileFactory = new jcz::TileFactory();


	int const N = 50;
	std::vector<Player *> players;
//	players.push_back(&RandomPlayer::instance);
//	players.push_back(&RandomPlayer::instance);
//	players.push_back(&RandomPlayer::instance);
//	players.push_back(&RandomPlayer::instance);
//	players.push_back(&RandomPlayer::instance);
//	players.push_back(new MonteCarloPlayer(tileFactory, false));
//	players.push_back(new MonteCarloPlayer<>(tileFactory));
	players.push_back(new MonteCarloPlayer<Utilities::SimpleUtility>(tileFactory));
	players.push_back(new MonteCarloPlayer<Utilities::HeydensUtility>(tileFactory));
//	players.push_back(new MonteCarloPlayer2<>(tileFactory));
//	players.push_back(new MonteCarloPlayer2(tileFactory, 2));
//	players.push_back(new MonteCarloPlayerUCT(tileFactory, false));
//	players.push_back(new MonteCarloPlayerUCT<>(tileFactory));
//	players.push_back(new MCTSPlayer<Utilities::SimpleUtility>(tileFactory));
//	players.push_back(new MCTSPlayer<Utilities::SimpleUtilityF>(tileFactory));
//	players.push_back(new MCTSPlayer<>(tileFactory));
//	players.push_back(new MCTSPlayer<Utilities::ComplexUtility, Playouts::EarlyCutoff<10>>(tileFactory));


#ifdef TIMEOUT
	qDebug() << "TIMEOUT" << TIMEOUT;
#endif
	for (Player * p : players)
		qDebug() << p->getTypeName();
	std::vector<Result> results;
	RandomNextTileProvider rntp;
	Game game(&rntp);
	int const playerCount = (int)players.size();
	Utilities::SimpleUtility utility;

	QElapsedTimer timer;
	QVarLengthArray<quint64, MAX_PLAYERS> times(playerCount);
	QVarLengthArray<qint64, MAX_PLAYERS> diffs(playerCount);
	QVarLengthArray<int, MAX_PLAYERS> steps(playerCount);
	for (int i = 0; i < playerCount; ++i)
	{
		times[i] = 0;
		diffs[i] = 0;
		steps[i] = 0;
	}
	QVarLengthArray<int, MAX_PLAYERS> playerAt(playerCount);
	for (int offset = 0; offset < playerCount; ++offset)
	{
		qDebug() << "round" << offset;
		for (int i = 0; i < N; ++i)
		{
			game.clearPlayers();
			for (int i = 0; i < playerCount; ++i)
			{
				int p = (i + offset) % playerCount;
				playerAt[i] = p;
				game.addPlayer(players[p]);
			}

			game.newGame(Tile::BaseGame, tileFactory);
			for (bool cont = true; cont; )
			{
				int player = game.getNextPlayer();

				timer.start();
				cont = game.step();
				qint64 elapsed = timer.nsecsElapsed();

				if (!game.getMoveHistory().back().move.tileMove.isNull())
				{
					times[playerAt[player]] += elapsed;
#ifdef TIMEOUT
					qint64 d = elapsed - qint64(1000000)*qint64(TIMEOUT);
#else
					qint64 d = elapsed;
#endif
					diffs[playerAt[player]] += d;
					++steps[playerAt[player]];
				}
				else
				{
					qDebug("skipped tile");
				}
			}

			Result result(playerCount);
			auto scores = game.getScores();
			auto utilities = utility.utilities(scores, playerCount);
			for (int i = 0; i < playerCount; ++i)
			{
				result.scores[playerAt[i]] = scores[i];
				result.utilities[playerAt[i]] = utilities[i];
				result.playouts[i] = players[i]->playouts;
			}

			results.push_back(std::move(result));

			printResults(results, playerCount);
			printTimes(times, diffs, steps, players);

			for (Player * p : players)
				p->playouts = 0;
			qDebug();
		}
	}

	delete tileFactory;
	return 0;
}
