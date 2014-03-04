#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "jcz/tilefactory.h"

#include <thread>
#include <chrono>
#include "player/randomplayer.h"
MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
	ui->setupUi(this);

	jcz::TileFactory * tileFactory = new jcz::TileFactory();
	boardUi = new BoardGraphicsScene(tileFactory, ui->boardView);
	game = new Game();

	boardUi->setGame(game);
	ui->boardView->setScene(boardUi);

	connect(boardUi, SIGNAL(sceneRectChanged(QRectF)), this, SLOT(recenter(QRectF)));

	Player * p1 = new RandomPlayer();
	Player * p2 = new RandomPlayer();

//	game->addPlayer(p1);
//	game->addPlayer(p2);
	game->addPlayer(this);
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

TileMove MainWindow::getTileMove(const Tile * const tile, const QList<Board::TilePlacement> & placements, const Game * const game)
{
	return boardUi->getMove(tile, placements, game);
}

MeepleMove MainWindow::getMeepleMove(const Tile * const tile, const QVarLengthArray<MeepleMove, NODE_ARRAY_LENGTH> & possible, const Game * const game)
{
	return boardUi->getMeepleMove(tile, possible, game);
}

void MainWindow::timeout()
{
	game->step();
	if (game->isFinished())
		timer->stop();
}

void MainWindow::recenter(QRectF rect)
{
	ui->boardView->centerOn(rect.center());
}
