#include <QCoreApplication>
#include <QTime>

#include "game.h"
#include "player/randomplayer.h"
#include "player/montecarloplayer.h"
#include "player/mctsplayer.h"
#include "util.h"

#define CONTROL_GAME 1
#include <iostream>
int main(int argc, char *argv[])
{
	QCoreApplication a(argc, argv);
	jcz::TileFactory * tileFactory = new jcz::TileFactory();
	RandomNextTileProvider rntp;
	Game * game = new Game(&rntp);
	
	if (false)
	{
		qDebug() << "CONTROL_GAME" << CONTROL_GAME;
		for (int i = 0; i < 1000; ++i)
		{
			qDebug() << "================================\nRUN" << i;
			Game g1(&rntp), g2(&rntp), g3(&rntp), g4(&rntp), g5(&rntp);
			Q_ASSERT(g1.equals(g2));
			Q_ASSERT(g2.equals(g1));
			for (int i = 0; i < 3; ++i)
			{
				g1.addPlayer(&RandomPlayer::instance);
				g2.addPlayer(&RandomPlayer::instance);
				g3.addPlayer(&RandomPlayer::instance);
				g4.addPlayer(&RandomPlayer::instance);
				g5.addPlayer(&RandomPlayer::instance);
			}
			Q_ASSERT(g1.equals(g2));
			Q_ASSERT(g2.equals(g1));
			g1.newGame(Tile::BaseGame, tileFactory);
			g2.newGame(Tile::BaseGame, tileFactory);
			g4.newGame(Tile::BaseGame, tileFactory);
			g5.newGame(Tile::BaseGame, tileFactory);
			Q_ASSERT(g1.equals(g2));
			Q_ASSERT(g2.equals(g1));
			
			int steps = 0;
			bool notDone = true;
			//for ( ; steps < 23; ++steps)
			for ( ; notDone; ++steps)
			{
				notDone = g1.step();
				MoveHistoryEntry const & e = g1.getMoveHistory().back();
				g4.simStep(e);

				g5.simPartStepChance(e.tile);
				g5.simPartStepTile(e.move.tileMove);
				g5.simPartStepMeeple(e.move.meepleMove);

				Q_ASSERT(g1.equals(g4));
				Q_ASSERT(g4.equals(g1));
				Q_ASSERT(g1.equals(g5));
				Q_ASSERT(g5.equals(g1));
#if CONTROL_GAME
				g3.newGame(Tile::BaseGame, tileFactory, g1.getMoveHistory());
				Q_ASSERT(g1.equals(g3));
				Q_ASSERT(g3.equals(g1));
#endif
			}
			for (int i = 0; i < steps; ++i)
			{
				g1.undo();
				g4.undo();

				g5.simPartUndoMeeple();
				g5.simPartUndoTile();
				g5.simPartUndoChance();

				Q_ASSERT(g1.equals(g4));
				Q_ASSERT(g4.equals(g1));
				Q_ASSERT(g1.equals(g5));
				Q_ASSERT(g5.equals(g1));
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
	

	if (false)
	{
		Player * p1 = &RandomPlayer::instance;
		MonteCarloPlayer * p2 = new MonteCarloPlayer(tileFactory);

		game->addPlayer(p1);
		game->addPlayer(p2);

		QTime t;
		int const n = 5;
		t.start();
		for (int i = 0; i < n; ++i)
		{
#if !COUNT_PLAYOUTS
			t.start();
#endif
			game->newGame(Tile::BaseGame, tileFactory);

			for (int ply = 0; game->step(); ++ply)
			{
//				std::cout << ply << std::endl;
			}
			int e = t.elapsed();
#if COUNT_PLAYOUTS
			std::cout << i << "   " << p2->playouts << "p / " << e << "ms = " << (p2->playouts) / (e / 1000.0) << " pps" << std::endl;
#else
			std::cout << i << "   " << e << std::endl;
#endif
		}
		std::cout << (t.elapsed() / n) << std::endl;
		return 0;
	}


	if (true)
	{
		Player * p1 = &RandomPlayer::instance;
		MCTSPlayer * p2 = new MCTSPlayer(tileFactory);

		game->addPlayer(p1);
		game->addPlayer(p2);

		QTime t;
		int const n = 5;
		t.start();
		for (int i = 0; i < n; ++i)
		{
#if !COUNT_PLAYOUTS
			t.start();
#endif
			game->newGame(Tile::BaseGame, tileFactory);

			for (int ply = 0; game->step(); ++ply)
			{
//				std::cout << ply << std::endl;
			}
			int e = t.elapsed();
#if COUNT_PLAYOUTS
			std::cout << i << "   " << p2->playouts << "p / " << e << "ms = " << (p2->playouts) / (e / 1000.0) << " pps" << std::endl;
#else
			std::cout << i << "   " << e << std::endl;
#endif
#if MCTS_COUNT_EXPAND_HITS
			std::cout << i << "   " << p2->hit << "hits / " << p2->miss << "misses = " << (p2->hit / p2->miss) << std::endl;
#endif
		}
		std::cout << (t.elapsed() / n) << std::endl;
		return 0;
	}

	return 0;
}
