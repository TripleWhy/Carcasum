#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "boardui.h"

#include <thread>
#include <chrono>
#include "player/randomplayer.h"
MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
	ui->setupUi(this);

	game = new Game();
	ui->scrollAreaWidgetContents->setGame(game);

	Player * p1 = new RandomPlayer();
	Player * p2 = new RandomPlayer();

	game->addPlayer(p1);
	game->addPlayer(p2);
	game->newGame(Tile::BaseGame);

//	new std::thread( [game]() {
//		while (!game->isFinished())
//		{
//			std::chrono::milliseconds dura( 500 );
//			std::this_thread::sleep_for( dura );
//			game->step();
//		}
//	} );

	timer = new QTimer(this);
	connect(timer, SIGNAL(timeout()), this, SLOT(timeout()));
	timer->setInterval(0);
	timer->setSingleShot(false);
	timer->start();
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
