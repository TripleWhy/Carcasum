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
#include <QDir>
#include <QDateTime>
#include <array>
#include <thread>

#if MAIN_USE_TEST_STATES
#include <QDir>
#endif

struct TestSetup
{
	QString text;
	std::vector<Player *> players;
	jcz::TileFactory * tileFactory;
	bool doIt;
	int N;
	bool printSteps;

	TestSetup(QString const text, std::vector<Player *> && players, jcz::TileFactory * tileFactory, bool const doIt = true, int const N = 104, bool const printSteps = true)
	    : text(text),
	      players(std::move(players)),
	      tileFactory(tileFactory),
	      doIt(doIt),
	      N(N),
	      printSteps(printSteps)
	{
	}

	TestSetup (TestSetup && other)
	    : text(other.text),
	      players(std::move(other.players)),
	      tileFactory(other.tileFactory),
	      doIt(other.doIt),
	      N(other.N),
	      printSteps(other.printSteps)
	{
	}

	TestSetup (const TestSetup & other) = delete;

	~TestSetup()
	{
		qDeleteAll(players);
		players.clear();
	}

	TestSetup & operator=(TestSetup && other) = default;
	TestSetup & operator=(TestSetup const & other) = delete;
};

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

void run(TestSetup const & setup, std::vector<Result> & results, int const threadId, int const threadCount, QString storeDir)
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
	for (Player const * p : setup.players)
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
		size_t from = threadId * setup.N / threadCount + offset * setup.N;
		size_t to = (threadId+1) * setup.N / threadCount + offset * setup.N;
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
			game.newGame(Tile::BaseGame, setup.tileFactory);
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
					if (setup.printSteps)
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

			if (setup.printSteps)
				printResult(results, playerCount, j);

#if MAIN_STORE_STATES
			QString file = QString("%1/%2_%3").arg(storeDir).arg(j, 3, 10, QLatin1Char('0')).arg(QDateTime::currentMSecsSinceEpoch());
			game.storeToFile(file);
#else
			Q_UNUSED(storeDir);
#endif
		}
	}

	qDeleteAll(players);
}

void doTest(TestSetup const & setup, int setupIndex)
{
#ifdef QT_NO_DEBUG
	int const THREADS = 8;
#else
	int const THREADS = 1;
#endif

#ifdef TIMEOUT
	qDebug() << "TIMEOUT" << TIMEOUT;
#endif
	qDebug() << "N" << setup.N;
	qDebug() << "THREADS" << THREADS;

	// feedback in case clone() does not work correctly:
	for (Player * p : setup.players)
	{
		auto clone = p->clone();
		qDebug() << clone->getTypeName();
		delete clone;
	}

	QString dir = QString("%1_games/setup_%2_%3").arg(qApp->applicationFilePath()).arg(setupIndex+1, 2, 10, QLatin1Char('0')).arg(setup.text);;
	QDir(dir).mkpath(".");
	if (setup.doIt)
	{
		std::vector<Result> results;
		results.resize(setup.N * setup.players.size());

		if (THREADS == 1)
		{
			run(setup, results, 0, THREADS, dir);
		}
		else
		{
			std::thread * threads[THREADS];
			for (int i = 0; i < THREADS; ++i)
				threads[i] = new std::thread(run, std::ref(setup), std::ref(results), i, THREADS, dir);
			for (int i = 0; i < THREADS; ++i)
				threads[i]->join();
		}
		printResults(results, (int)setup.players.size());
		qDebug("\n");
	}
}

#include <iostream>
int main(int argc, char *argv[])
{
	QCoreApplication app(argc, argv);
	qDebug() << "Qt build version:  " << QT_VERSION_STR;
	qDebug() << "Qt runtime version:" << qVersion();
	qDebug() << "Git revision:" << APP_REVISION_STR;
#ifdef RANDOM_SEED
	qDebug("WARNING: RANDOM_SEED is set: " STR(RANDOM_SEED));
#endif

	std::vector<TestSetup> setups;
	jcz::TileFactory * tileFactory = new jcz::TileFactory(false);
	std::vector<Player *> players;

//	if (false)
//	{
//		players.push_back(new MCTSPlayer<>(tileFactory));
//		players.push_back(new MCTSPlayer1<>(tileFactory));
//		setups.emplace_back( "MCTSPlayer vs MCTSPlayer1", std::move(players), tileFactory );
//	}
//	if (false)
//	{
//		players.push_back(new MCTSPlayer<Utilities::PortionUtility, Playouts::RandomPlayout>(tileFactory));
//		players.push_back(new MCTSPlayerMT<Utilities::PortionUtility, Playouts::RandomPlayout>(tileFactory));
//		setup.emplace_back( "MCTSPlayer vs MCTSPlayerMT", std::move(players, tileFactory) );
//	}
	if (false) // simple players
	{
		players.push_back(new SimplePlayer());
		players.push_back(new RandomPlayer());
		setups.emplace_back( "SimplePlayer vs RandomPlayer", std::move(players), tileFactory, true, 10000, false );

		players.push_back(new SimplePlayer());
		players.push_back(new SimplePlayer2());
		setups.emplace_back( "SimplePlayer vs SimplePlayer2", std::move(players), tileFactory, true, 10000, false );

		players.push_back(new SimplePlayer());
		players.push_back(new SimplePlayer3());
		setups.emplace_back( "SimplePlayer vs SimplePlayer3", std::move(players), tileFactory, true, 10000, false );

		players.push_back(new SimplePlayer());
		players.push_back(new jcz::JCZPlayer(tileFactory));
		setups.emplace_back( "SimplePlayer vs JCZPlayer", std::move(players), tileFactory, true, 1000, false );

		players.push_back(new SimplePlayer2());
		players.push_back(new SimplePlayer3());
		setups.emplace_back( "SimplePlayer2 vs SimplePlayer3", std::move(players), tileFactory, true, 10000, false );

		players.push_back(new SimplePlayer2());
		players.push_back(new jcz::JCZPlayer(tileFactory));
		setups.emplace_back( "SimplePlayer2 vs JCZPlayer", std::move(players), tileFactory, true, 1000, false );

		players.push_back(new SimplePlayer3());
		players.push_back(new jcz::JCZPlayer(tileFactory));
		setups.emplace_back( "SimplePlayer3 vs JCZPlayer", std::move(players), tileFactory, true, 1000, false );
	}
	if (false) //roulette wheel players
	{
		players.push_back(new RandomPlayer());
		players.push_back(new RouletteWheelPlayer());
		setups.emplace_back( "RandomPlayer vs RouletteWheelPlayer", std::move(players), tileFactory, true, 10000, false );

		players.push_back(new SimplePlayer3());
		players.push_back(new RouletteWheelPlayer());
		setups.emplace_back( "SimplePlayer3 vs RouletteWheelPlayer", std::move(players), tileFactory, true, 10000, false );

		players.push_back(new RandomPlayer());
		players.push_back(new RouletteWheelPlayer2());
		setups.emplace_back( "RandomPlayer vs RouletteWheelPlayer2", std::move(players), tileFactory, true, 10000, false );

		players.push_back(new SimplePlayer3());
		players.push_back(new RouletteWheelPlayer2());
		setups.emplace_back( "SimplePlayer3 vs RouletteWheelPlayer2", std::move(players), tileFactory, true, 10000, false );
	}
	if (false)	//skip
	{
		players.push_back(new UtilityPlayer1<Utilities::HeydensEvaluation>(tileFactory));
		players.push_back(new SimplePlayer());
		setups.emplace_back( "", std::move(players), tileFactory, true, 10000, false );

		players.push_back(new UtilityPlayer1<Utilities::HeydensEvaluation>(tileFactory));
		players.push_back(new SimplePlayer2());
		setups.emplace_back( "", std::move(players), tileFactory, true, 10000, false );

		players.push_back(new UtilityPlayer1<Utilities::HeydensEvaluation>(tileFactory));
		players.push_back(new SimplePlayer3());
		setups.emplace_back( "", std::move(players), tileFactory, true, 10000, false );

		players.push_back(new UtilityPlayer1<Utilities::HeydensEvaluation>(tileFactory));
		players.push_back(new jcz::JCZPlayer(tileFactory));
		setups.emplace_back( "", std::move(players), tileFactory, true, 1000, false );

		players.push_back(new UtilityPlayer2<Utilities::HeydensEvaluation>(tileFactory));
		players.push_back(new SimplePlayer());
		setups.emplace_back( "", std::move(players), tileFactory, true, 10000, false );

		players.push_back(new UtilityPlayer2<Utilities::HeydensEvaluation>(tileFactory));
		players.push_back(new SimplePlayer2());
		setups.emplace_back( "", std::move(players), tileFactory, true, 10000, false );

		players.push_back(new UtilityPlayer2<Utilities::HeydensEvaluation>(tileFactory));
		players.push_back(new SimplePlayer3());
		setups.emplace_back( "", std::move(players), tileFactory, true, 10000, false );

		players.push_back(new UtilityPlayer2<Utilities::HeydensEvaluation>(tileFactory));
		players.push_back(new jcz::JCZPlayer(tileFactory));
		setups.emplace_back( "", std::move(players), tileFactory, true, 1000, false );
	}
	if (false) // Monte Carlo vs simple players
	{
		players.push_back(new MonteCarloPlayer<>(tileFactory, 50, true));
		players.push_back(new RandomPlayer());
		setups.emplace_back( " MonteCarloPlayer_50ms vs RandomPlayer", std::move(players), tileFactory );

		players.push_back(new MonteCarloPlayer<>(tileFactory));
		players.push_back(new SimplePlayer3);
		setups.emplace_back( "MonteCarloPlayer vs SimplePlayer3", std::move(players), tileFactory );

		players.push_back(new MonteCarloPlayer<>(tileFactory));
		players.push_back(new jcz::JCZPlayer(tileFactory));
		setups.emplace_back( "MonteCarloPlayer vs JCZPlayer", std::move(players), tileFactory );
	}
	if (false) // Monte Carlo variants
	{
		players.push_back(new MonteCarloPlayer<>(tileFactory));
		players.push_back(new MonteCarloPlayer2<>(tileFactory));
		setups.emplace_back( "MonteCarloPlayer vs MonteCarloPlayer2", std::move(players), tileFactory );

		players.push_back(new MonteCarloPlayer<>(tileFactory));
		players.push_back(new MonteCarloPlayerUCT<>(tileFactory));
		setups.emplace_back( "MonteCarloPlayer vs MonteCarloPlayerUCT", std::move(players), tileFactory );
	}
	if (false) // Monte Carlo vs others
	{
		players.push_back(new jcz::JCZPlayer(tileFactory));
		players.push_back(new MCTSPlayer<>(tileFactory));
		setups.emplace_back( "MonteCarloPlayer vs MCTSPlayer", std::move(players), tileFactory );

		players.push_back(new MonteCarloPlayer<>(tileFactory));
		players.push_back(new MCTSPlayer<>(tileFactory));
		setups.emplace_back( "MonteCarloPlayer vs MCTSPlayer", std::move(players), tileFactory );

		players.push_back(new MonteCarloPlayer2<>(tileFactory));
		players.push_back(new MCTSPlayer<>(tileFactory));
		setups.emplace_back( "MonteCarloPlayer2 vs MCTSPlayer", std::move(players), tileFactory );

		players.push_back(new MonteCarloPlayerUCT<>(tileFactory));
		players.push_back(new MCTSPlayer<>(tileFactory));
		setups.emplace_back( "MonteCarloPlayerUCT vs MCTSPlayer", std::move(players), tileFactory );
	}
	if (false) // different utilities
	{
		players.push_back(new MCTSPlayer<Utilities::ComplexUtility, Playouts::RandomPlayout>(tileFactory));
		players.push_back(new MCTSPlayer<Utilities::SimpleUtility, Playouts::RandomPlayout>(tileFactory));
		setups.emplace_back( "ComplexUtility vs SimpleUtility", std::move(players), tileFactory );

		players.push_back(new MCTSPlayer<Utilities::ComplexUtilityNormalized, Playouts::RandomPlayout>(tileFactory));
		players.push_back(new MCTSPlayer<Utilities::SimpleUtility, Playouts::RandomPlayout>(tileFactory));
		setups.emplace_back( "ComplexUtilityNormalized vs SimpleUtility", std::move(players), tileFactory );

		players.push_back(new MCTSPlayer<Utilities::Normalized<Utilities::ComplexUtility>, Playouts::RandomPlayout>(tileFactory));
		players.push_back(new MCTSPlayer<Utilities::SimpleUtility, Playouts::RandomPlayout>(tileFactory));
		setups.emplace_back( "Normalized<ComplexUtility> vs SimpleUtility", std::move(players), tileFactory );

		players.push_back(new MCTSPlayer<Utilities::ScoreDifferenceUtility, Playouts::RandomPlayout>(tileFactory));
		players.push_back(new MCTSPlayer<Utilities::SimpleUtility, Playouts::RandomPlayout>(tileFactory));
		setups.emplace_back( "ScoreDifferenceUtility vs SimpleUtility", std::move(players), tileFactory );

		players.push_back(new MCTSPlayer<Utilities::Normalized<Utilities::ScoreDifferenceUtility>, Playouts::RandomPlayout>(tileFactory));
		players.push_back(new MCTSPlayer<Utilities::SimpleUtility, Playouts::RandomPlayout>(tileFactory));
		setups.emplace_back( "Normalized<ScoreDifferenceUtility> vs SimpleUtility", std::move(players), tileFactory );

		players.push_back(new MCTSPlayer<Utilities::NormalizedOld<Utilities::ScoreDifferenceUtility>, Playouts::RandomPlayout>(tileFactory));
		players.push_back(new MCTSPlayer<Utilities::SimpleUtility, Playouts::RandomPlayout>(tileFactory));
		setups.emplace_back( "NormalizedOld<ScoreDifferenceUtility> vs SimpleUtility", std::move(players), tileFactory );

		players.push_back(new MCTSPlayer<Utilities::NormalizedNeg<Utilities::ScoreDifferenceUtility>, Playouts::RandomPlayout>(tileFactory));
		players.push_back(new MCTSPlayer<Utilities::SimpleUtility, Playouts::RandomPlayout>(tileFactory));
		setups.emplace_back( "NormalizedNeg<ScoreDifferenceUtility> vs SimpleUtility", std::move(players), tileFactory );

		players.push_back(new MCTSPlayer<Utilities::Normalized<Utilities::HeydensEvaluation>, Playouts::RandomPlayout>(tileFactory));
		players.push_back(new MCTSPlayer<Utilities::SimpleUtility, Playouts::RandomPlayout>(tileFactory));
		setups.emplace_back( "Normalized<HeydensEvaluation> vs SimpleUtility", std::move(players), tileFactory );

		players.push_back(new MCTSPlayer<Utilities::PortionUtility, Playouts::RandomPlayout>(tileFactory));
		players.push_back(new MCTSPlayer<Utilities::SimpleUtility, Playouts::RandomPlayout>(tileFactory));
		setups.emplace_back( "PortionUtility vs SimpleUtility", std::move(players), tileFactory );

		players.push_back(new MCTSPlayer<Utilities::Bonus<Utilities::PortionUtility, 100>, Playouts::RandomPlayout>(tileFactory));
		players.push_back(new MCTSPlayer<Utilities::SimpleUtility, Playouts::RandomPlayout>(tileFactory));
		setups.emplace_back( "Bonus<PortionUtility, 100> vs SimpleUtility", std::move(players), tileFactory );

		players.push_back(new MCTSPlayer<Utilities::Bonus<Utilities::ComplexUtility, 1>, Playouts::RandomPlayout>(tileFactory));
		players.push_back(new MCTSPlayer<Utilities::SimpleUtility, Playouts::RandomPlayout>(tileFactory));
		setups.emplace_back( "Bonus<ComplexUtility, 1> vs SimpleUtility", std::move(players), tileFactory );
	}
	if (false) // Test 2
	{
		for (int i = 0; i < 15; ++i)
		{
			int p1 = (1 << i);
			int p2 = (2 << i);
			players.push_back(new MCTSPlayer<Utilities::SimpleUtility, Playouts::RandomPlayout>(tileFactory, false, p1, false, 1.0));
			players.push_back(new MCTSPlayer<Utilities::SimpleUtility, Playouts::RandomPlayout>(tileFactory, false, p2, false, 1.0));
			setups.emplace_back( QString("Test 2: %1 vs %2").arg(p1).arg(p2), std::move(players), tileFactory );
		}
	}
	if (false) // Cp values
	{
		qreal const Cp2 = 0.5;
		qreal const Cps[] = {0.0, 0.25, 0.50, 0.75, 1.0, 2.0, 3.0};
		for (qreal const Cp : Cps)
		{
			players.push_back(new MCTSPlayer<Utilities::PortionUtility, Playouts::RandomPlayout>(tileFactory, false, TIMEOUT, true, Cp));
			players.push_back(new MCTSPlayer<Utilities::PortionUtility, Playouts::RandomPlayout>(tileFactory, false, TIMEOUT, true, Cp2));
			setups.emplace_back( QString("Cp %1 vs Cp %2").arg(Cp).arg(Cp2), std::move(players), tileFactory );
		}
	}

	// playout policies
	if (false) // early cutoff value
	{
		players.push_back(new MCTSPlayer<Utilities::PortionUtility, Playouts::RandomPlayout>    (tileFactory, false));
		players.push_back(new MCTSPlayer<Utilities::EC<Utilities::PortionUtility>, Playouts::EarlyCutoff<0>>(tileFactory, false));
		setups.emplace_back( "Early Cutoff 0", std::move(players), tileFactory );

		players.push_back(new MCTSPlayer<Utilities::PortionUtility, Playouts::RandomPlayout>    (tileFactory, false));
		players.push_back(new MCTSPlayer<Utilities::EC<Utilities::PortionUtility>, Playouts::EarlyCutoff<4>>(tileFactory, false));
		setups.emplace_back( "Early Cutoff 4", std::move(players), tileFactory );

		players.push_back(new MCTSPlayer<Utilities::PortionUtility, Playouts::RandomPlayout>    (tileFactory, false));
		players.push_back(new MCTSPlayer<Utilities::EC<Utilities::PortionUtility>, Playouts::EarlyCutoff<8>>(tileFactory, false));
		setups.emplace_back( "Early Cutoff 8", std::move(players), tileFactory );

		players.push_back(new MCTSPlayer<Utilities::PortionUtility, Playouts::RandomPlayout>    (tileFactory, false));
		players.push_back(new MCTSPlayer<Utilities::EC<Utilities::PortionUtility>, Playouts::EarlyCutoff<16>>(tileFactory, false));
		setups.emplace_back( "Early Cutoff 16", std::move(players), tileFactory );
	}
	if (false) // e-greedy value
	{
		players.push_back(new MCTSPlayer<Utilities::PortionUtility, Playouts::RandomPlayout>    (tileFactory, false));
		players.push_back(new MCTSPlayer<Utilities::PortionUtility, Playouts::EGreedy<20>>(tileFactory, false));
		setups.emplace_back( "MCTSPlayer vs MCTSPlayer greedy s3 20", std::move(players), tileFactory );

		players.push_back(new MCTSPlayer<Utilities::PortionUtility, Playouts::RandomPlayout>    (tileFactory, false));
		players.push_back(new MCTSPlayer<Utilities::PortionUtility, Playouts::EGreedy<80>>(tileFactory, false));
		setups.emplace_back( "MCTSPlayer vs MCTSPlayer greedy s3 80", std::move(players), tileFactory );

		players.push_back(new MCTSPlayer<Utilities::PortionUtility, Playouts::RandomPlayout>    (tileFactory, false));
		players.push_back(new MCTSPlayer<Utilities::PortionUtility, Playouts::EGreedy<50>>(tileFactory, false));
		setups.emplace_back( "MCTSPlayer vs MCTSPlayer greedy s3 50", std::move(players), tileFactory );

		players.push_back(new MCTSPlayer<Utilities::PortionUtility, Playouts::RandomPlayout>    (tileFactory, false));
		players.push_back(new MCTSPlayer<Utilities::PortionUtility, Playouts::EGreedy<0>>(tileFactory, false));
		setups.emplace_back( "MCTSPlayer vs MCTSPlayer greedy s3 0", std::move(players), tileFactory );

		players.push_back(new MCTSPlayer<Utilities::PortionUtility, Playouts::RandomPlayout>    (tileFactory, false));
		players.push_back(new MCTSPlayer<Utilities::PortionUtility, Playouts::EGreedy<100>>(tileFactory, false));
		setups.emplace_back( "MCTSPlayer vs MCTSPlayer greedy s3 100", std::move(players), tileFactory );
	}
	if (false) // roulette wheel selection
	{
		qDebug() << "SIMPLE_PLAYER3_NEGATIVE_SCORE_HANDLE_VARIANT" << SIMPLE_PLAYER3_NEGATIVE_SCORE_HANDLE_VARIANT;
		players.push_back(new MCTSPlayer<Utilities::PortionUtility, Playouts::RandomPlayout>    (tileFactory, false));
		players.push_back(new MCTSPlayer<Utilities::PortionUtility, Playouts::PlayerPlayout<RouletteWheelPlayer>>(tileFactory, false));
		setups.emplace_back( "MCTSPlayer vs MCTSPlayer RWS1", std::move(players), tileFactory );

		players.push_back(new MCTSPlayer<Utilities::PortionUtility, Playouts::RandomPlayout>    (tileFactory, false));
		players.push_back(new MCTSPlayer<Utilities::PortionUtility, Playouts::PlayerPlayout<RouletteWheelPlayer2>>(tileFactory, false));
		setups.emplace_back( "MCTSPlayer vs MCTSPlayer RWS2", std::move(players), tileFactory );
	}

	// MCTS enhancements
	if (false) // tree reusage
	{
		players.push_back(new MCTSPlayer<Utilities::PortionUtility, Playouts::RandomPlayout>(tileFactory, true));
		players.push_back(new MCTSPlayer<Utilities::PortionUtility, Playouts::RandomPlayout>(tileFactory, false));
		setups.emplace_back( "MCTSPlayer reuseTree vs MCTSPlayer plain", std::move(players), tileFactory );
	}
	if (false) // node priors
	{
		players.push_back(new MCTSPlayer<>(tileFactory, false, TIMEOUT, true, 0.5, false));
		players.push_back(new MCTSPlayer<>(tileFactory, false, TIMEOUT, true, 0.5, true));
		setups.emplace_back( "Node Priors", std::move(players), tileFactory );

		players.push_back(new MCTSPlayer<>                                                   (tileFactory, false, TIMEOUT, true, 0.5, false));
		players.push_back(new MCTSPlayer<Utilities::Normalized<Utilities::HeydensEvaluation>>(tileFactory, false, TIMEOUT, true, 0.5, true));
		setups.emplace_back( "Node Priors using HeydensEvaluation", std::move(players), tileFactory );
	}
	if (false) // progressive widening
	{
		players.push_back(new MCTSPlayer<>(tileFactory, false, TIMEOUT, true, 0.5, false, false));
		players.push_back(new MCTSPlayer<>(tileFactory, false, TIMEOUT, true, 0.5, false, true));
		setups.emplace_back( "Progressive Widening", std::move(players), tileFactory );
	}
	if (false)	// progressive bias
	{
		players.push_back(new MCTSPlayer<Utilities::Normalized<Utilities::HeydensEvaluation>>(tileFactory, false, TIMEOUT, true, 0.5, false, false, false));
		players.push_back(new MCTSPlayer<Utilities::Normalized<Utilities::HeydensEvaluation>>(tileFactory, false, TIMEOUT, true, 0.5, false, false, true));
		setups.emplace_back( "Progressive Bias", std::move(players), tileFactory );
	}


	if (setups.size() == 0)
	{
		qDebug("nothing to do");
		return 0;
	}

	int from = 0;
	int to = (int)setups.size() - 1;

	QStringList const & args = app.arguments();
	if (args.size() > 1)
	{
		bool ok;
		int i = args[1].toInt(&ok);
		if (ok)
		{
			from = i - 1;
			to =  from;
			if (args.size() > 2)
			{
				i = args[2].toInt(&ok);
				if (ok)
					to = i - 1;
			}
		}
	}
	if (from < 0)
		from = 0;
	if (from > (int)setups.size() -1)
		from = (int)setups.size() -1;
	if (to < 0)
		to = 0;
	if (to > (int)setups.size() -1)
		to = (int)setups.size() -1;
	if (to < from)
		to = from;

	qDebug() << "Running tests" << (from+1) << "-" << (to+1) << "of" << setups.size();

	setups.erase(setups.begin() + to + 1, setups.end());
	setups.erase(setups.begin(), setups.begin() + from);

	QElapsedTimer totalTime;
	QElapsedTimer time;
	totalTime.start();

	int index = from;
	for (auto const & setup : setups)
	{
		time.start();
		qDebug("\n");
		qDebug() << setup.text;
		doTest(setup, index++);
		qDebug() << "Time:" << time.elapsed();
	}
	qDebug() << "\nTotal Time:" << totalTime.elapsed();

	delete tileFactory;
	return 0;
}
