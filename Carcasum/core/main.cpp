/*
	This file is part of Carcasum.

	Carcasum is free software: you can redistribute it and/or modify
	it under the terms of the GNU Affero General Public License as published by
	the Free Software Foundation, either version 3 of the License, or
	(at your option) any later version.

	Carcasum is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU Affero General Public License for more details.

	You should have received a copy of the GNU Affero General Public License
	along with Carcasum.  If not, see <http://www.gnu.org/licenses/>.
*/

#include <QCoreApplication>
#include <QTime>

#include "game.h"
#include "player/randomplayer.h"
#include "player/montecarloplayer.h"
#include "player/montecarloplayer2.h"
#include "player/mctsplayer.h"
#include "player/simpleplayer.h"
#include "jcz/jczplayer.h"

#define CONTROL_GAME 1
#include <iostream>
int main(int argc, char *argv[])
{
	QCoreApplication a(argc, argv);
	jcz::TileFactory * tileFactory = new jcz::TileFactory();
	RandomNextTileProvider rntp;
	Game * game = new Game(&rntp);
	
	qDebug("NODE_VARIANT: " STR(NODE_VARIANT));
	if (false)
	{
		qDebug() << "CONTROL_GAME" << CONTROL_GAME;
		for (int i = 0; i < 10000; ++i)
		{
			qDebug() << "================================\nRUN" << i;
			Game g1(&rntp), g2(&rntp), g3(&rntp), g4(&rntp), g5(&rntp);
			Q_ASSERT(g1.equals(g2));
			Q_ASSERT(g2.equals(g1));

			g1.addPlayer(&RandomPlayer::instance);
			g1.addPlayer(&RandomPlayer::instance);
//			g1.addPlayer(new jcz::JCZPlayer(tileFactory));

			for (Player * p : g1.getPlayers())
			{
				g2.addPlayer(p->clone());
				g3.addPlayer(p->clone());
				g4.addPlayer(p->clone());
				g5.addPlayer(p->clone());
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

				g5.simPartStepChance(e.tileIndex);
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
				g1.simUndo();
				g4.simUndo();

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

	if (true)
	{
		static int const playouts = 5000;
		Game g(&rntp);
		g.addPlayer(&RandomPlayer::instance);
		g.addPlayer(&RandomPlayer::instance);
//		g.addPlayer(new jcz::JCZPlayer(tileFactory));
//		g.addPlayer(new jcz::JCZPlayer(tileFactory));
//		g.addPlayer(new SimplePlayer());

		QTime t;
		t.start();
		for (int i = 0; i < playouts; ++i)
		{
			g.newGame(Tile::BaseGame, tileFactory);
			int steps = 0;
			do
			{
				++steps;
			} while (g.step());
		}
		int e = t.elapsed();
		std::cout << playouts << "p / " << e << "ms = " << playouts / (e / 1000.0) << " pps" << std::endl;
//		return 0;
	}

	if (true)
	{
		static int const playouts = 5000;
		Game g(&rntp);
		g.addPlayer(&RandomPlayer::instance);
		g.addPlayer(&RandomPlayer::instance);
//		g.addPlayer(new SimplePlayer());
		g.newGame(Tile::BaseGame, tileFactory);

		QTime t;
		t.start();
		for (int i = 0; i < playouts; ++i)
		{
			int steps = 0;
			do
			{
				++steps;
			} while (g.step());

			for (; steps > 0; --steps)
			{
				g.simUndo();
			}
		}
		int e = t.elapsed();
		std::cout << playouts << "p / " << e << "ms = " << playouts / (e / 1000.0) << " pps" << std::endl;
		return 0;
	}
	

	if (false)
	{
		Player * p1 = &RandomPlayer::instance;
//		auto * p2 = new MonteCarloPlayer<>(tileFactory);
		auto * p2 = new MonteCarloPlayer2<>(tileFactory);

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
		auto * p2 = new MCTSPlayer<>(tileFactory);

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
//				std::cout << "ply " << ply << std::endl;
			}
			int e = t.elapsed();
#if COUNT_PLAYOUTS
			std::cout << i << "   " << p2->playouts << "p / " << e << "ms = " << (p2->playouts) / (e / 1000.0) << " pps" << std::endl;
#else
			std::cout << i << "   " << e << std::endl;
#endif
#if MCTS_COUNT_EXPAND_HITS
			std::cout << i << "   " << p2->hit << "hits / " << p2->miss << "misses = " << (p2->hit / qreal(p2->miss)) << std::endl;
#endif
		}
		std::cout << (qreal(t.elapsed()) / qreal(n)) << std::endl;
		return 0;
	}

	if (false)
	{
		SimplePlayer3 s3;
		game->addPlayer(&s3);
		game->newGame(Tile::BaseGame, tileFactory);

		forever
		{
			game->simStep(&RandomPlayer::instance);
			if (game->getPlayerMeeples(0) <= 0)
				break;
		}
		qDebug() << game->getPlayerMeeples(0);

		forever
		{
			game->simStep(&s3);
			if (game->getPlayerMeeples(0) > 0)
				break;
		}
		qDebug() << game->getPlayerMeeples(0);

		qDebug();
		MoveHistoryEntry h = game->getMoveHistory().back();
		game->undo();
		qDebug() << "m" << game->getPlayerMeeples(0);
//		qDebug() << "r" << game->getPlayerReturnMeeples(0);
		game->simStep(h);
		qDebug() << "m" << game->getPlayerMeeples(0);
//		qDebug() << "r" << game->getPlayerReturnMeeples(0);
		game->undo();
		qDebug() << "m" << game->getPlayerMeeples(0);
//		qDebug() << "r" << game->getPlayerReturnMeeples(0);

		qDebug();
		qDebug("simPartStepChance");
		game->simPartStepChance(h.tileIndex);
		qDebug() << "m" << game->getPlayerMeeples(0);
//		qDebug() << "r" << game->getPlayerReturnMeeples(0);
		qDebug("simPartStepTile");
		game->simPartStepTile(h.move.tileMove);
		qDebug() << "m" << game->getPlayerMeeples(0);
//		qDebug() << "r" << game->getPlayerReturnMeeples(0);
		qDebug() << "placements" << game->getPossibleMeeplePlacements(0, game->simTile).size();
		qDebug("simPartStepMeeple");
		game->simPartStepMeeple(h.move.meepleMove);
		qDebug() << "m" << game->getPlayerMeeples(0);
//		qDebug() << "r" << game->getPlayerReturnMeeples(0);

		qDebug();
		qDebug();
		qDebug("simPartUndoMeeple");
		game->simPartUndoMeeple();
		qDebug() << "m" << game->getPlayerMeeples(0);
//		qDebug() << "r" << game->getPlayerReturnMeeples(0);
		qDebug() << "placements" << game->getPossibleMeeplePlacements(0, game->simTile).size();
		qDebug("simPartUndoTile");
		game->simPartUndoTile();
		qDebug() << "m" << game->getPlayerMeeples(0);
//		qDebug() << "r" << game->getPlayerReturnMeeples(0);
		qDebug("simPartUndoChance");
		game->simPartUndoChance();
		qDebug() << "m" << game->getPlayerMeeples(0);
//		qDebug() << "r" << game->getPlayerReturnMeeples(0);
	}

	return 0;
}
