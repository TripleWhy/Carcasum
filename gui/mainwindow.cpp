#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "jcz/tilefactory.h"

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

	game->addWatchingPlayer(boardUi);
	
	game->addPlayer(p1);
	game->addPlayer(p1);
	game->addPlayer(p1);
	game->addPlayer(p1);
	game->addPlayer(p1);
//	game->addPlayer(p1);
	game->addPlayer(boardUi);
	game->newGame(Tile::BaseGame, tileFactory);

	new std::thread( [this]() {
		while (!game->isFinished())
		{
			Util::sleep(500);
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
