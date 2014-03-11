#include <QCoreApplication>
#include <QTime>

#include "game.h"
#include "player/randomplayer.h"
#include "player/montecarloplayer.h"
#include "util.h"

#include <iostream>

int main(int argc, char *argv[])
{
	QCoreApplication a(argc, argv);
	
	jcz::TileFactory * tileFactory = new jcz::TileFactory();
	Game * game = new Game();
	
	Player * p1 = &RandomPlayer::instance;
	MonteCarloPlayer * p2 = new MonteCarloPlayer(tileFactory);

	game->addPlayer(p1);
	game->addPlayer(p2);
	
	QTime t;
	int const n = 5;
	double sum = 0;
	for (int i = 0; i < n; ++i)
	{
		game->newGame(Tile::BaseGame, tileFactory);
	
		t.start();
		while (!game->isFinished())
		{
			game->step();
//			std::cout << game->getPly() << std::endl;
		}
		int e = t.elapsed();
		sum += e;
		std::cout << e << std::endl;
		std::cout << p2->t1 << "\t" << p2->t2 << "\t" << p2->t3 << "\t" << std::endl;
		std::cout << p2->t4 << "\t" << p2->t5 << "\t" << p2->t6 << "\t" << std::endl;
		std::cout << (p2->t1 + p2->t4) << "\t" << (p2->t2 + p2->t5) << "\t" << (p2->t3 + p2->t6) << "\t" << std::endl;
		p2->t1 = p2->t2 = p2->t3 = p2->t4 = p2->t5 = p2->t6 = 0;
	}
	std::cout << (sum / n) << std::endl;

	return 0;
}
