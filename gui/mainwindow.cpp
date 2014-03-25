#include "mainwindow.h"
#include "ui_mainwindow.h"

#include "playerinfoview.h"
#include "jcz/tilefactory.h"

#include "player/randomplayer.h"
#include "player/montecarloplayer.h"
MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
	ui->setupUi(this);

	boardUi = new BoardGraphicsScene(&tileFactory, &imgFactory, ui->boardView);
	game = new Game();

	boardUi->setGame(game);
	ui->boardView->setScene(boardUi);

	connect(boardUi, SIGNAL(sceneRectChanged(QRectF)), this, SLOT(recenter(QRectF)));

	Player * p1 = &RandomPlayer::instance;
	Player * p2 = new MonteCarloPlayer(&tileFactory);

	game->addWatchingPlayer(this);
	
	game->addPlayer(p1);
//	game->addPlayer(p1);
//	game->addPlayer(p1);
//	game->addPlayer(p1);
//	game->addPlayer(p1);
//	game->addPlayer(p1);
	game->addPlayer(p2);
	game->addPlayer(this);
	game->newGame(Tile::BaseGame, &tileFactory);

	new std::thread( [this]() {
		while (!game->isFinished())
		{
			Util::sleep(500);
			game->step();
		}
	} );

//	timer = new QTimer(this);
//	connect(timer, SIGNAL(timeout()), this, SLOT(timeout()));
//	timer->setInterval(500);
//	timer->setSingleShot(false);
//	timer->start();
}

MainWindow::~MainWindow()
{
	delete ui;
}

void MainWindow::newGame(int player, const Game * const game)
{
	if (playerInfos.size() != game->getPlayerCount())
	{
		qDeleteAll(playerInfos);
		playerInfos.clear();
		QVBoxLayout * l = ui->playerInfoLayout;
		for (uint i = 0; i < game->getPlayerCount(); ++i)
		{
			PlayerInfoView * pi = new PlayerInfoView(i, game, &imgFactory);
			l->insertWidget(i, pi);
			connect(this, SIGNAL(updateNeeded()), pi, SLOT(updateView()));
			playerInfos.push_back(pi);
		}
	}
	
	boardUi->newGame(player, game);
}

void MainWindow::playerMoved(int player, const Tile * const tile, const MoveHistoryEntry & move)
{
	emit updateNeeded();
	boardUi->playerMoved(player, tile, move);
}

TileMove MainWindow::getTileMove(int player, const Tile * const tile, const MoveHistoryEntry & move, const TileMovesType & placements)
{
	return boardUi->getTileMove(player, tile, move, placements);
}

MeepleMove MainWindow::getMeepleMove(int player, const Tile * const tile, const MoveHistoryEntry & move, const MeepleMovesType & possible)
{
	return boardUi->getMeepleMove(player, tile, move, possible);
}

void MainWindow::endGame()
{
	emit updateNeeded();
	return boardUi->endGame();
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
