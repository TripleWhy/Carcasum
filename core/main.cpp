#include <QCoreApplication>
#include <QTime>

#include "game.h"
#include "player/randomplayer.h"
#include "player/montecarloplayer.h"
#include "util.h"

#define CONTROL_GAME 1
#include <iostream>
int main(int argc, char *argv[])
{
	QCoreApplication a(argc, argv);
	jcz::TileFactory * tileFactory = new jcz::TileFactory();
	Game * game = new Game();
	
	if (false)
	{
		for (int i = 0; i < 1000; ++i)
		{
			Game g1, g2, g3;
			Q_ASSERT(g1.equals(g2));
			Q_ASSERT(g2.equals(g1));
			for (int i = 0; i < 3; ++i)
			{
				g1.addPlayer(&RandomPlayer::instance);
				g2.addPlayer(&RandomPlayer::instance);
				g3.addPlayer(&RandomPlayer::instance);
			}
			Q_ASSERT(g1.equals(g2));
			Q_ASSERT(g2.equals(g1));
			g1.newGame(Tile::BaseGame, tileFactory);
			g2.newGame(Tile::BaseGame, tileFactory);
			Q_ASSERT(g1.equals(g2));
			Q_ASSERT(g2.equals(g1));
			
			int steps = 0;
			//for ( ; steps < 23; ++steps)
			for ( ; !g1.isFinished(); ++steps)
			{
				g1.step();
#if CONTROL_GAME
				g3.newGame(Tile::BaseGame, tileFactory, g1.getMoveHistory());
				Q_ASSERT(g1.equals(g3));
				Q_ASSERT(g3.equals(g1));
#endif
			}
			for (int i = 0; i < steps; ++i)
			{
				g1.undo();
#if CONTROL_GAME
				g3.newGame(Tile::BaseGame, tileFactory, g1.getMoveHistory());
				Q_ASSERT(g1.equals(g3));
				Q_ASSERT(g3.equals(g1));
#endif
			}
			Q_ASSERT(g1.equals(g2));
			Q_ASSERT(g2.equals(g1));
		}
		
		return 0;
	}
	
	
	Player * p1 = &RandomPlayer::instance;
	Player * p2 = new MonteCarloPlayer(tileFactory);

	game->addPlayer(p1);
	game->addPlayer(p2);
	
	QTime t;
	int const n = 5;
	double sum = 0;
	for (int i = 0; i < n; ++i)
	{
		game->newGame(Tile::BaseGame, tileFactory);
	
		t.start();
		for (int ply = 0; !game->isFinished(); ++ply)
		{
			game->step();
			std::cout << ply << std::endl;
		}
		int e = t.elapsed();
		sum += e;
		std::cout << e << std::endl;
	}
	std::cout << (sum / n) << std::endl;

	return 0;
}
