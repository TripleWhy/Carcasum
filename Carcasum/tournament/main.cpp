#include "static.h"
#include "core/game.h"
#include "player/randomplayer.h"
#include "player/montecarloplayer.h"
#include "player/montecarloplayer2.h"
#include "player/montecarloplayeruct.h"
#include "player/mctsplayer.h"
//#include "player/mctsplayer1.h"
//#include "player/mctsplayermt.h"
#include "player/simpleplayer.h"
#include "player/simpleplayer2.h"
#include "player/simpleplayer3.h"
#include "player/specialplayers.h"
#include "jcz/jczplayer.h"
#include <QCoreApplication>
#include <QElapsedTimer>
#include <QVarLengthArray>
#include <array>
#include <thread>

#if MAIN_USE_TEST_STATES
#include <QDir>
#endif

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

void run(std::vector<Player *> const & players_, jcz::TileFactory * tileFactory, std::vector<Result> & results, int const N, int const threadId, int const threadCount, bool const printSteps)
{
#ifdef Q_OS_UNIX
	// The follwing code ties one thread to one core on linux.
	cpu_set_t cpuset;
	CPU_ZERO(&cpuset);
	CPU_SET(threadId, &cpuset);
	sched_setaffinity(0, sizeof(cpu_set_t), &cpuset);
#endif

#if MAIN_USE_TEST_STATES
	QDir dir("states/");
	QStringList filters;
	filters << "state*";
	QStringList const & files = dir.entryList(filters);
	if (files.size() == 0)
	{
		qWarning("files empty");
		exit(1);
	}
#endif

	std::vector<Player *> players;
	for (Player const * p : players_)
		players.push_back(p->clone());

	RandomNextTileProvider rntp;
#if MAIN_USE_TEST_STATES
	HistoryProvider htp(&rntp);
	Game game(&htp);
#else
	Game game(&rntp);
#endif
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

#if MAIN_USE_TEST_STATES
			QString const & file = dir.filePath(files[ (int)j % files.size() ]);
			qDebug() << j << file;
			auto && history = Game::loadFromFile(file);
			htp.setData(history, history.size() - 1);
			history.pop_back();
			game.newGame(Tile::BaseGame, tileFactory, history, true);
#else
			game.newGame(Tile::BaseGame, tileFactory);
#endif
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
					if (printSteps)
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

			if (printSteps)
				printResult(results, playerCount, j);
		}
	}

	qDeleteAll(players);
}

void doTest(std::vector<Player *> & players, jcz::TileFactory * tileFactory, bool const doIt = true, int const N=100, bool const printSteps = true)
{
#ifdef QT_NO_DEBUG
	int const THREADS = 8;
#else
	int const THREADS = 1;
#endif

#ifdef TIMEOUT
	qDebug() << "TIMEOUT" << TIMEOUT;
#endif
	qDebug() << "N" << N;
	qDebug() << "THREADS" << THREADS;

	// feedback in case clone() does not work correctly:
	for (Player * p : players)
	{
		auto clone = p->clone();
		qDebug() << clone->getTypeName();
		delete clone;
	}

	if (doIt)
	{
		std::vector<Result> results;
		results.resize(N * players.size());

		if (THREADS == 1)
		{
			run(players, tileFactory, std::ref(results), N, 0, THREADS, printSteps);
		}
		else
		{
			std::thread * threads[THREADS];
			for (int i = 0; i < THREADS; ++i)
				threads[i] = new std::thread(run, players, tileFactory, std::ref(results), N, i, THREADS, printSteps);
			for (int i = 0; i < THREADS; ++i)
				threads[i]->join();
		}
		printResults(results, (int)players.size());
		qDebug("\n");
	}

	qDeleteAll(players);
	players.clear();
}

#include <iostream>
int main(int /*argc*/, char */*argv*/[])
{
	qDebug() << "Qt build version:  " << QT_VERSION_STR;
	qDebug() << "Qt runtime version:" << qVersion();
	qDebug() << "Git revision:" << APP_REVISION_STR;
#ifdef RANDOM_SEED
	qDebug("WARNING: RANDOM_SEED is set: " STR(RANDOM_SEED));
#endif

	jcz::TileFactory * tileFactory = new jcz::TileFactory(false);
	std::vector<Player *> players;
	QElapsedTimer totalTime;
	totalTime.start();

//	if (false)
//	{
//		qDebug("\n\nMCTSPlayer vs MCTSPlayer1");
//		players.push_back(new MCTSPlayer<>(tileFactory));
//		players.push_back(new MCTSPlayer1<>(tileFactory));
//		doTest(players, tileFactory);
//	}
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
	if (false)
	{
		qDebug("\n\nMCTSPlayer reuseTree vs MCTSPlayer plain ");

		players.push_back(new MCTSPlayer<Utilities::PortionUtility, Playouts::RandomPlayout>(tileFactory, true));
		players.push_back(new MCTSPlayer<Utilities::PortionUtility, Playouts::RandomPlayout>(tileFactory, false));
		doTest(players, tileFactory);
	}
	if (false)
	{
		qDebug();
		players.push_back(new UtilityPlayer1<Utilities::HeydensEvaluation>(tileFactory));
		players.push_back(new SimplePlayer());
		doTest(players, tileFactory, true, 10000, false);

		qDebug();
		players.push_back(new UtilityPlayer1<Utilities::HeydensEvaluation>(tileFactory));
		players.push_back(new SimplePlayer2());
		doTest(players, tileFactory, true, 10000, false);

		qDebug();
		players.push_back(new UtilityPlayer1<Utilities::HeydensEvaluation>(tileFactory));
		players.push_back(new SimplePlayer3());
		doTest(players, tileFactory, true, 10000, false);

		qDebug();
		players.push_back(new UtilityPlayer1<Utilities::HeydensEvaluation>(tileFactory));
		players.push_back(new jcz::JCZPlayer(tileFactory));
		doTest(players, tileFactory, true, 100, false);

		qDebug();
		players.push_back(new UtilityPlayer2<Utilities::HeydensEvaluation>(tileFactory));
		players.push_back(new SimplePlayer());
		doTest(players, tileFactory, true, 10000, false);

		qDebug();
		players.push_back(new UtilityPlayer2<Utilities::HeydensEvaluation>(tileFactory));
		players.push_back(new SimplePlayer2());
		doTest(players, tileFactory, true, 10000, false);

		qDebug();
		players.push_back(new UtilityPlayer2<Utilities::HeydensEvaluation>(tileFactory));
		players.push_back(new SimplePlayer3());
		doTest(players, tileFactory, true, 10000, false);

		qDebug();
		players.push_back(new UtilityPlayer2<Utilities::HeydensEvaluation>(tileFactory));
		players.push_back(new jcz::JCZPlayer(tileFactory));
		doTest(players, tileFactory, true, 100, false);

		qDebug("-------------------");

		qDebug();
		players.push_back(new SimplePlayer());
		players.push_back(new SimplePlayer2());
		doTest(players, tileFactory, true, 10000, false);

		qDebug();
		players.push_back(new SimplePlayer());
		players.push_back(new SimplePlayer3());
		doTest(players, tileFactory, true, 10000, false);

		qDebug();
		players.push_back(new SimplePlayer());
		players.push_back(new jcz::JCZPlayer(tileFactory));
		doTest(players, tileFactory, true, 1000, false);

		qDebug();
		players.push_back(new SimplePlayer2());
		players.push_back(new SimplePlayer3());
		doTest(players, tileFactory, true, 10000, false);

		qDebug();
		players.push_back(new SimplePlayer2());
		players.push_back(new jcz::JCZPlayer(tileFactory));
		doTest(players, tileFactory, true, 1000, false);

		qDebug();
		players.push_back(new SimplePlayer3());
		players.push_back(new jcz::JCZPlayer(tileFactory));
		doTest(players, tileFactory, true, 1000, false);

		qDebug();
		players.push_back(new SimplePlayer3());
		players.push_back(new RouletteWheelPlayer());
		doTest(players, tileFactory, true, 10000, false);

		qDebug();
		players.push_back(new SimplePlayer3());
		players.push_back(new RouletteWheelPlayer2());
		doTest(players, tileFactory, true, 10000, false);
	}
//	if (false)
//	{
//		qDebug("\n\nMCTSPlayer vs MCTSPlayerMT");
//		players.push_back(new MCTSPlayer<Utilities::PortionUtility, Playouts::RandomPlayout>(tileFactory));
//		players.push_back(new MCTSPlayerMT<Utilities::PortionUtility, Playouts::RandomPlayout>(tileFactory));
//		doTest(players, tileFactory);
//	}
	if (false)
	{
		qreal const Cps[] = {0.0, 0.25, 0.50, 0.75, 1.0, 2.0, 3.0};
		for (qreal const Cp : Cps)
		{
			qDebug() << "\n\nCp" << Cp;
			players.push_back(new MCTSPlayer<Utilities::PortionUtility, Playouts::RandomPlayout>(tileFactory, false, TIMEOUT, true, Cp));
			players.push_back(new MCTSPlayer<Utilities::PortionUtility, Playouts::RandomPlayout>(tileFactory, false, TIMEOUT, true, 0.5));
			doTest(players, tileFactory);
		}
	}
	if (false)
	{
		qDebug("\n\nMCTSPlayer random vs MCTSPlayer simple2 20");
		players.push_back(new MCTSPlayer<Utilities::PortionUtility, Playouts::RandomPlayout>    (tileFactory, false));
		players.push_back(new MCTSPlayer<Utilities::PortionUtility, Playouts::EGreedy<20>>(tileFactory, false));
		doTest(players, tileFactory);

		qDebug("\n\nMCTSPlayer random vs MCTSPlayer simple2 80");
		players.push_back(new MCTSPlayer<Utilities::PortionUtility, Playouts::RandomPlayout>    (tileFactory, false));
		players.push_back(new MCTSPlayer<Utilities::PortionUtility, Playouts::EGreedy<80>>(tileFactory, false));
		doTest(players, tileFactory);

		qDebug("\n\nMCTSPlayer random vs MCTSPlayer simple2 50");
		players.push_back(new MCTSPlayer<Utilities::PortionUtility, Playouts::RandomPlayout>    (tileFactory, false));
		players.push_back(new MCTSPlayer<Utilities::PortionUtility, Playouts::EGreedy<50>>(tileFactory, false));
		doTest(players, tileFactory);

		qDebug("\n\nMCTSPlayer random vs MCTSPlayer simple2 0");
		players.push_back(new MCTSPlayer<Utilities::PortionUtility, Playouts::RandomPlayout>    (tileFactory, false));
		players.push_back(new MCTSPlayer<Utilities::PortionUtility, Playouts::EGreedy<0>>(tileFactory, false));
		doTest(players, tileFactory);

		qDebug("\n\nMCTSPlayer random vs MCTSPlayer simple2 100");
		players.push_back(new MCTSPlayer<Utilities::PortionUtility, Playouts::RandomPlayout>    (tileFactory, false));
		players.push_back(new MCTSPlayer<Utilities::PortionUtility, Playouts::EGreedy<100>>(tileFactory, false));
		doTest(players, tileFactory);
	}
	if (false)
	{
		qDebug() << "SIMPLE_PLAYER3_NEGATIVE_SCORE_HANDLE_VARIANT" << SIMPLE_PLAYER3_NEGATIVE_SCORE_HANDLE_VARIANT;
		qDebug("\n\nMCTSPlayer random vs MCTSPlayer RWS1");
		players.push_back(new MCTSPlayer<Utilities::PortionUtility, Playouts::RandomPlayout>    (tileFactory, false));
		players.push_back(new MCTSPlayer<Utilities::PortionUtility, Playouts::PlayerPlayout<RouletteWheelPlayer>>(tileFactory, false));
		doTest(players, tileFactory, true);

		qDebug("\n\nMCTSPlayer random vs MCTSPlayer RWS2");
		players.push_back(new MCTSPlayer<Utilities::PortionUtility, Playouts::RandomPlayout>    (tileFactory, false));
		players.push_back(new MCTSPlayer<Utilities::PortionUtility, Playouts::PlayerPlayout<RouletteWheelPlayer2>>(tileFactory, false));
		doTest(players, tileFactory, true);
	}
	if (false)
	{
		QElapsedTimer t;
		qDebug("\n\n");
		players.push_back(new MCTSPlayer<Utilities::SimpleUtility, Playouts::RandomPlayout>  (tileFactory, false));
		players.push_back(new MCTSPlayer<Utilities::Normalized<Utilities::HeydensEvaluation>>(tileFactory, false));
		t.start();
		doTest(players, tileFactory, true);
		qDebug() << "Time:" <<t.elapsed();

		qDebug("\n\n");
		players.push_back(new MCTSPlayer<Utilities::PortionUtility, Playouts::RandomPlayout> (tileFactory, false));
		players.push_back(new MCTSPlayer<Utilities::Normalized<Utilities::HeydensEvaluation>>(tileFactory, false));
		t.start();
		doTest(players, tileFactory, true);
		qDebug() << "Time:" <<t.elapsed();

		qDebug("\n\n");
		players.push_back(new MCTSPlayer<>(tileFactory, false, TIMEOUT, true, 0.5, false));
		players.push_back(new MCTSPlayer<>(tileFactory, false, TIMEOUT, true, 0.5, true));
		t.start();
		doTest(players, tileFactory, true);
		qDebug() << "Time:" <<t.elapsed();

		qDebug("\n\n");
		players.push_back(new MCTSPlayer<Utilities::PortionUtility, Playouts::RandomPlayout> (tileFactory, false, TIMEOUT, true, 0.5, false));
		players.push_back(new MCTSPlayer<Utilities::Normalized<Utilities::HeydensEvaluation>>(tileFactory, false, TIMEOUT, true, 0.5, true));
		t.start();
		doTest(players, tileFactory, true);
		qDebug() << "Time:" <<t.elapsed();
	}
	if (true)
	{
		qDebug("\n\nProgressive Widening");
		players.push_back(new MCTSPlayer<>(tileFactory, false, TIMEOUT, true, 0.5, false, false));
		players.push_back(new MCTSPlayer<>(tileFactory, false, TIMEOUT, true, 0.5, false, true));
		doTest(players, tileFactory, true);

		qDebug("\n\nProgressive Bias");
		players.push_back(new MCTSPlayer<Utilities::Normalized<Utilities::HeydensEvaluation>>(tileFactory, false, 100, true, 0.5, false, false, false));
		players.push_back(new MCTSPlayer<Utilities::Normalized<Utilities::HeydensEvaluation>>(tileFactory, false, 100, true, 0.5, false, false, true));
		doTest(players, tileFactory, true, 50);
	}

	qDebug() << "\nTotal Time:" << totalTime.elapsed();

	delete tileFactory;
	return 0;
}
