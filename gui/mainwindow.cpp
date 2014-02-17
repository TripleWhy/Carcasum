#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "core/game.h"
#include "boardui.h"

#include <thread>
#include <chrono>
#include "player/randomplayer.h"
MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
	ui->setupUi(this);

	Game * game = new Game();
	ui->scrollAreaWidgetContents->setGame(game);

	Player * p1 = new RandomPlayer();
	Player * p2 = new RandomPlayer();

	game->addPlayer(p1);
	game->addPlayer(p2);
	game->newGame(Tile::BaseGame);

	new std::thread( [game]() {
		while (!game->isFinished())
		{
			std::chrono::milliseconds dura( 1000 );
			std::this_thread::sleep_for( dura );
			game->step();
		}
	} );
}

MainWindow::~MainWindow()
{
	delete ui;
}
