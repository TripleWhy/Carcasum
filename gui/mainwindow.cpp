#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "boardui.h"
#include "jcz/jczutils.h"

#include <thread>
#include <chrono>
#include "player/randomplayer.h"
MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
	ui->setupUi(this);
	JCZUtils::TileFactory * tileFactory = new JCZUtils::TileFactory();

	game = new Game();
	ui->boardUi->setGame(game);
	ui->boardUi->setTileFactory(tileFactory);

	Player * p1 = new RandomPlayer();
	Player * p2 = new RandomPlayer();

	game->addPlayer(p1);
	game->addPlayer(p2);
//	game->addPlayer(this);
	game->newGame(Tile::BaseGame, tileFactory);

	new std::thread( [this]() {
		while (!game->isFinished())
		{
			std::chrono::milliseconds dura( 500 );
			std::this_thread::sleep_for( dura );
			game->step();
		}
	} );

//	timer = new QTimer(this);
//	connect(timer, SIGNAL(timeout()), this, SLOT(timeout()));
//	timer->setInterval(0);
//	timer->setSingleShot(false);
//	timer->start();
}

MainWindow::~MainWindow()
{
	delete ui;
}

Move MainWindow::getMove(const Tile * const tile, const QList<Board::TilePlacement> & placements, const Game * const game)
{
	return ui->boardUi->getMove(tile, placements, game);
}

void MainWindow::timeout()
{
	game->step();
	if (game->isFinished())
		timer->stop();
}
