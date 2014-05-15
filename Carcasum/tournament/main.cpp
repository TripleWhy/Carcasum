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
#include <array>
#include <thread>

struct Result
{
	QVarLengthArray<int, MAX_PLAYERS> scores;
	QVarLengthArray<int, MAX_PLAYERS> utilities;
	QVarLengthArray<quint64, MAX_PLAYERS> playouts;
	QVarLengthArray<quint64, MAX_PLAYERS> times;
	QVarLengthArray<qint64, MAX_PLAYERS> timeDiffs;
	QVarLengthArray<int, MAX_PLAYERS> plys;

	Result(int size = 0)
	    : scores(size),
	      utilities(size),
	      playouts(size),
	      times(size),
	      timeDiffs(size),
	      plys(size)
	{}
};

template<typename T>
void printResults(T const & results, int const playerCount)
{
	QVarLengthArray<int, MAX_PLAYERS> wins(playerCount);
	QVarLengthArray<int, MAX_PLAYERS> draws(playerCount);
	QVarLengthArray<int, MAX_PLAYERS> scores(playerCount);
	QVarLengthArray<quint64, MAX_PLAYERS> playouts(playerCount);
	QVarLengthArray<int, MAX_PLAYERS> plys(playerCount);
	QVarLengthArray<quint64, MAX_PLAYERS> times(playerCount);
	QVarLengthArray<qint64, MAX_PLAYERS> timeDiffs(playerCount);
	for (int i = 0; i < playerCount; ++i)
	{
		wins[i] = 0;
		draws[i] = 0;
		scores[i] = 0;
		playouts[i] = 0;
		plys[i] = 0;
		times[i] = 0;
		timeDiffs[i] = 0;
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
			plys[i] += r.plys[i];
			times[i] += r.times[i];
			timeDiffs[i] += r.timeDiffs[i];
		}
	}

	size_t const size = results.size();
	qreal const dSize = qreal(size);
	for (int i = 0; i < playerCount; ++i)
	{
		qDebug() << "player" << i
		         << "   wins:" << wins[i] << "/" << size << "=" << (wins[i] / dSize)
		         << "   draws:" << draws[i] << "/" << size << "=" << (draws[i] / dSize)
//		         << "\n"
		         << "\tavg. score:" << (scores[i] / dSize)
		         << "\tavg. playouts:" << (qreal(playouts[i]) / dSize)
		         << "\tavg. plys:" << (plys[i] / dSize) << "(" << (qreal(playouts[i]) / plys[i]) << "ppp )"
		         << "\tavg. time:" << (qreal(times[i]) / (dSize*1000000)) << "(" << (qreal(playouts[i]*1000*1000000) / qreal(times[i])) << "pps )"
		         << "\tavg. overrun:" << (qreal(timeDiffs[i]) / qreal(qint64(plys[i])*1000000))
		   ;
	}
}

template<typename T>
void printResult(T const & results, int const playerCount, size_t index)
{
	QDebug && d = qDebug();
	for (int i = 0; i < playerCount; ++i)
	{
		Result const & r = results[index];
		auto _score = r.scores[i];
		auto _playouts = r.playouts[i];
		auto _plys = r.plys[i];
		auto _time = r.times[i];
		d.nospace() << index << " player " << i
		  << "   score: " << _score
		  << "   playouts: " << _playouts
		  << "   plys: " << _plys << " (" << (qreal(_playouts) / qreal(_plys)) << " ppp)"
		  << "   time: " << (_time/1000000) << " (" << (qreal(_playouts*1000*1000000) / qreal(_time)) << " pps)"
		  << "\n";
	}
}

void run(std::vector<Player *> const & players_, jcz::TileFactory * tileFactory, std::vector<Result> & results, int const N, int const threadId, int const threadCount)
{
#ifdef Q_OS_UNIX
	// The follwing code ties one thread to one core on linux.
	//-->
	cpu_set_t cpuset;
	CPU_ZERO(&cpuset);
	CPU_SET(threadId, &cpuset);
	sched_setaffinity(0, sizeof(cpu_set_t), &cpuset);
	//<--
#endif

	std::vector<Player *> players;
	for (Player const * p : players_)
		players.push_back(p->clone());

	RandomNextTileProvider rntp;
	Game game(&rntp);
	int const playerCount = (int)players.size();
	Utilities::SimpleUtility utility;

	QElapsedTimer timer;
	QVarLengthArray<int, MAX_PLAYERS> playerAt(playerCount);
	for (int offset = 0; offset < playerCount; ++offset)
	{
		size_t from = threadId * N / threadCount + offset * N;
		size_t to = (threadId+1) * N / threadCount + offset * N;
		for (size_t j = from; j < to; ++j)
		{
			game.clearPlayers();
			for (int i = 0; i < playerCount; ++i)
			{
				int p = (i + offset) % playerCount;
				playerAt[i] = p;
				game.addPlayer(players[p]);
			}

			Result result(playerCount);
			for (int i = 0; i < playerCount; ++i)
			{
				result.times[i] = 0;
				result.timeDiffs[i] = 0;
				result.plys[i] = 0;
			}
			game.newGame(Tile::BaseGame, tileFactory);
			int i = 0;
			for (bool cont = true; cont; ++i)
			{
				qint64 elapsed;
				int player = game.getNextPlayer();

				timer.start();
				cont = game.step();
				elapsed = timer.nsecsElapsed();

				if (!game.getMoveHistory().back().move.tileMove.isNull())
				{
					result.times[playerAt[player]] += elapsed;
#ifdef TIMEOUT
					qint64 d = elapsed - qint64(1000000)*qint64(TIMEOUT);
#else
					qint64 d = elapsed;
#endif
					result.timeDiffs[playerAt[player]] += d;
					++result.plys[playerAt[player]];
				}
				else
				{
					qDebug() << i << "skipped tile";
				}
			}

			auto scores = game.getScores();
			auto utilities = utility.utilities(scores, playerCount, &game);
			for (int i = 0; i < playerCount; ++i)
			{
				result.scores[playerAt[i]] = scores[i];
				result.utilities[playerAt[i]] = utilities[i];
#if COUNT_PLAYOUTS
				result.playouts[i] = players[i]->playouts;
				players[i]->playouts = 0;
#endif
			}

			results[j] = result;

			printResult(results, playerCount, j);
		}
	}

	qDeleteAll(players);
}

void doTest(std::vector<Player *> & players, jcz::TileFactory * tileFactory)
{
	int const N = 100;
	int const THREADS = 8;

#ifdef TIMEOUT
	qDebug() << "TIMEOUT" << TIMEOUT;
#endif
	// feedback in case clone() does not work correctly:
	for (Player * p : players)
	{
		auto clone = p->clone();
		qDebug() << clone->getTypeName();
		delete clone;
	}

	if (true)
	{
		std::vector<Result> results;
		results.resize(N * players.size());

		if (THREADS == 1)
		{
			run(players, tileFactory, std::ref(results), N, 0, THREADS);
		}
		else
		{
			std::thread * threads[THREADS];
			for (int i = 0; i < THREADS; ++i)
				threads[i] = new std::thread(run, players, tileFactory, std::ref(results), N, i, THREADS);
			for (int i = 0; i < THREADS; ++i)
				threads[i]->join();
		}
		printResults(results, (int)players.size());
		qDebug("\n");
	}

	qDeleteAll(players);
	players.clear();
}

int main(int /*argc*/, char */*argv*/[])
{
	qDebug() << "Qt build version:  " << QT_VERSION_STR;
	qDebug() << "Qt runtime version:" << qVersion();
	qDebug() << "Git revision:" << APP_REVISION_STR;

	jcz::TileFactory * tileFactory = new jcz::TileFactory(false);
	std::vector<Player *> players;

	if (false)
	{
		qDebug("\n\nComplexUtility vs SimpleUtility");
		players.push_back(new MCTSPlayer<Utilities::ComplexUtility, Playouts::RandomPlayout>(tileFactory));
		players.push_back(new MCTSPlayer<Utilities::SimpleUtility, Playouts::RandomPlayout>(tileFactory));
		doTest(players, tileFactory);

		qDebug("\n\nComplexUtilityNormalized vs SimpleUtility");
		players.push_back(new MCTSPlayer<Utilities::ComplexUtilityNormalized, Playouts::RandomPlayout>(tileFactory));
		players.push_back(new MCTSPlayer<Utilities::SimpleUtility, Playouts::RandomPlayout>(tileFactory));
		doTest(players, tileFactory);

		qDebug("\n\nNormalized<ComplexUtility> vs SimpleUtility");
		players.push_back(new MCTSPlayer<Utilities::Normalized<Utilities::ComplexUtility>, Playouts::RandomPlayout>(tileFactory));
		players.push_back(new MCTSPlayer<Utilities::SimpleUtility, Playouts::RandomPlayout>(tileFactory));
		doTest(players, tileFactory);

		qDebug("\n\nHeydensUtility vs SimpleUtility");
		players.push_back(new MCTSPlayer<Utilities::HeydensUtility, Playouts::RandomPlayout>(tileFactory));
		players.push_back(new MCTSPlayer<Utilities::SimpleUtility, Playouts::RandomPlayout>(tileFactory));
		doTest(players, tileFactory);

		qDebug("\n\nNormalized<HeydensUtility> vs SimpleUtility");
		players.push_back(new MCTSPlayer<Utilities::Normalized<Utilities::HeydensUtility>, Playouts::RandomPlayout>(tileFactory));
		players.push_back(new MCTSPlayer<Utilities::SimpleUtility, Playouts::RandomPlayout>(tileFactory));
		doTest(players, tileFactory);

		qDebug("\n\nPortionUtility vs SimpleUtility");
		players.push_back(new MCTSPlayer<Utilities::PortionUtility, Playouts::RandomPlayout>(tileFactory));
		players.push_back(new MCTSPlayer<Utilities::SimpleUtility, Playouts::RandomPlayout>(tileFactory));
		doTest(players, tileFactory);

		qDebug("\n\nBonus<PortionUtility, 100> vs SimpleUtility");
		players.push_back(new MCTSPlayer<Utilities::Bonus<Utilities::PortionUtility, 100>, Playouts::RandomPlayout>(tileFactory));
		players.push_back(new MCTSPlayer<Utilities::SimpleUtility, Playouts::RandomPlayout>(tileFactory));
		doTest(players, tileFactory);

		qDebug("\n\nBonus<ComplexUtility, 1> vs SimpleUtility");
		players.push_back(new MCTSPlayer<Utilities::Bonus<Utilities::ComplexUtility, 1>, Playouts::RandomPlayout>(tileFactory));
		players.push_back(new MCTSPlayer<Utilities::SimpleUtility, Playouts::RandomPlayout>(tileFactory));
		doTest(players, tileFactory);
	}
	if (false)
	{
		qDebug("\n\nTest 2");
		for (int i = 14; i < 15; ++i)
		{
			int p1 = (1 << i);
			int p2 = (2 << i);
			qDebug() << "\n" << p1 << "vs" << p2;
			players.push_back(new MCTSPlayer<Utilities::SimpleUtility, Playouts::RandomPlayout>(tileFactory, p1, false, 1.0));
			players.push_back(new MCTSPlayer<Utilities::SimpleUtility, Playouts::RandomPlayout>(tileFactory, p2, false, 1.0));
			doTest(players, tileFactory);
		}
	}
	if (false)
	{
		qDebug("\n\nMonteCarloPlayer vs MCTSPlayer");
		players.push_back(new MonteCarloPlayer<Utilities::PortionUtility, Playouts::RandomPlayout>(tileFactory));
		players.push_back(new MCTSPlayer<Utilities::PortionUtility, Playouts::RandomPlayout>(tileFactory, false));
		doTest(players, tileFactory);

		qDebug("\n\nMonteCarloPlayer2 vs MCTSPlayer");
		players.push_back(new MonteCarloPlayer2<Utilities::PortionUtility, Playouts::RandomPlayout>(tileFactory));
		players.push_back(new MCTSPlayer<Utilities::PortionUtility, Playouts::RandomPlayout>(tileFactory, false));
		doTest(players, tileFactory);

		qDebug("\n\nMonteCarloPlayerUCT vs MCTSPlayer");
		players.push_back(new MonteCarloPlayerUCT<Utilities::PortionUtility, Playouts::RandomPlayout>(tileFactory));
		players.push_back(new MCTSPlayer<Utilities::PortionUtility, Playouts::RandomPlayout>(tileFactory, false));
		doTest(players, tileFactory);
	}
	if (true)
	{
		qDebug("\n\nMCTSPlayer plain vs MCTSPlayer reuseTree");
		players.push_back(new MCTSPlayer<Utilities::PortionUtility, Playouts::RandomPlayout>(tileFactory, false));
		players.push_back(new MCTSPlayer<Utilities::PortionUtility, Playouts::RandomPlayout>(tileFactory, true));
		doTest(players, tileFactory);
	}

	delete tileFactory;
	return 0;
}
