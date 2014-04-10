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

	Result(int size) : scores(size), utilities(size) {}
};

void printResults(std::vector<Result> const & results, int const playerCount)
{
	QVarLengthArray<int, MAX_PLAYERS> wins(playerCount);
	QVarLengthArray<int, MAX_PLAYERS> draws(playerCount);
	QVarLengthArray<int, MAX_PLAYERS> scores(playerCount);
	for (int i = 0; i < playerCount; ++i)
	{
		wins[i] = 0;
		draws[i] = 0;
		scores[i] = 0;
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
		}
	}

	for (int i = 0; i < playerCount; ++i)
	{
		qDebug() << "player" << i << "   wins:" << wins[i] << "/" << results.size() << "=" << (wins[i] / qreal(results.size()))
		         <<"   draws:" << draws[i] << "/" << results.size() << "=" << (draws[i] / qreal(results.size()))
		         << "   avg. score:" << (scores[i] / qreal(results.size()));
	}
}

void printTimes(QVarLengthArray<quint64, MAX_PLAYERS> const & times, QVarLengthArray<qint64, MAX_PLAYERS> const & diffs, QVarLengthArray<int, MAX_PLAYERS> const & steps, std::vector<Player *> const & players)
{
	for (int i = 0; i < times.size(); ++i)
	{
		qreal s = qreal(quint64(steps[i]) * quint64(1000000));
		Player * p = players[i];
		qDebug() << "player" << i << "   playouts:" << p->playouts << "\tavg. thinking time:" << (times[i] / s) << "ms   avg. overrun:" << (diffs[i] / s) << "ms";
		p->playouts = 0;
	}
}

int main(int argc, char *argv[])
{
	QCoreApplication app(argc, argv);
	jcz::TileFactory * tileFactory = new jcz::TileFactory();


	int const N = 50;
	std::vector<Player *> players;
//	players.push_back(new MonteCarloPlayer(tileFactory, false));
//	players.push_back(new MonteCarloPlayer(tileFactory, true));
	players.push_back(new MonteCarloPlayer2(tileFactory, false));
//	players.push_back(new MonteCarloPlayer2(tileFactory, true));
	players.push_back(new MonteCarloPlayerUCT(tileFactory, false));
//	players.push_back(new MonteCarloPlayerUCT(tileFactory, true));


#ifdef TIMEOUT
	qDebug() << "TIMEOUT" << TIMEOUT;
#endif
	std::vector<Result> results;
	RandomNextTileProvider rntp;
	Game game(&rntp);
	int const playerCount = players.size();

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
					qint64 d = elapsed - qint64(1000000)*qint64(TIMEOUT);
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
			auto utilities = Util::utilitySimpleMulti(scores, playerCount);
			for (int i = 0; i < playerCount; ++i)
			{
				result.scores[playerAt[i]] = scores[i];
				result.utilities[playerAt[i]] = utilities[i];
			}

			results.push_back(std::move(result));

			printResults(results, playerCount);
			printTimes(times, diffs, steps, players);
			qDebug();
		}
	}

	delete tileFactory;
	return 0;
}
