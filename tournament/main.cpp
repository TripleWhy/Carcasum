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

int main(int argc, char *argv[])
{
	QCoreApplication app(argc, argv);
	jcz::TileFactory * tileFactory = new jcz::TileFactory();


	int const N = 25;
	std::vector<Player *> players;
//	players.push_back(new MonteCarloPlayer(tileFactory, false));
	players.push_back(new MonteCarloPlayer(tileFactory, true));
//	players.push_back(new MonteCarloPlayer2(tileFactory, false));
//	players.push_back(new MonteCarloPlayer2(tileFactory, true));
	players.push_back(new MonteCarloPlayerUCT(tileFactory, true));


#ifdef TIMEOUT
	qDebug() << "TIMEOUT" << TIMEOUT;
#endif
	std::vector<Result> results;
	RandomNextTileProvider rntp;
	Game game(&rntp);
	int const playerCount = players.size();

	QElapsedTimer timer;
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
			qreal tileCount = game.getTileCount();

			timer.start();
			while (game.step())
			{}
			int elapsed = timer.elapsed();

			qDebug() << "playout took" << elapsed << "ms =>" << (elapsed / tileCount) << "ms per ply";
			Result result(playerCount);
			auto scores = game.getScores();
			for (int i = 0; i < playerCount; ++i)
				result.scores[playerAt[i]] = scores[i];
			result.utilities = Util::utilitySimpleMulti(scores, playerCount);

			results.push_back(std::move(result));

			printResults(results, playerCount);
			qDebug();
		}
	}

	delete tileFactory;
	return 0;
}
