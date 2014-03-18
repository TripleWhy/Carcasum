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
	t.start();
	for (int i = 0; i < n; ++i)
	{
		game->newGame(Tile::BaseGame, tileFactory);
	
		for (int ply = 0; !game->isFinished(); ++ply)
		{
			game->step();
			std::cout << ply << std::endl;
		}
		int e = t.elapsed();
		std::cout << i << "   " << p2->playouts << "p / " << e << "ms = " << (p2->playouts) / (e / 1000.0) << " pps" << std::endl;
	}
	std::cout << (t.elapsed() / n) << std::endl;

	return 0;
}
