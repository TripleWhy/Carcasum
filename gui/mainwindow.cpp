#include "mainwindow.h"
#include "ui_mainwindow.h"

#include "playerinfoview.h"
#include "jcz/tilefactory.h"

#include <QActionGroup>
#include <QSettings>

#include "player/randomplayer.h"
#include "player/montecarloplayer.h"
#include "player/montecarloplayer2.h"
#include "player/mctsplayer.h"
MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
	ui->setupUi(this);
	auto actionGroup = new QActionGroup(this);
	actionGroup->addAction(ui->actionRandom_Tiles);
	actionGroup->addAction(ui->actionChoose_Tiles);

	boardUi = new BoardGraphicsScene(&tileFactory, &imgFactory, ui->boardView);
	game = new Game(&rntp);

	boardUi->setGame(game);
	ui->boardView->setScene(boardUi);

	connect(boardUi, SIGNAL(sceneRectChanged(QRectF)), this, SLOT(recenter(QRectF)));

	readSettings();

	Player * p1 = &RandomPlayer::instance;
	Player * p2 = new MonteCarloPlayer<>(&tileFactory);
	Player * p4 = new MonteCarloPlayer<Utilities::SimpleUtility>(&tileFactory);
	Player * p5 = new MonteCarloPlayer2<>(&tileFactory);
	Player * p3 = new MCTSPlayer<>(&tileFactory);

	game->addWatchingPlayer(this);
	
//	game->addPlayer(p1);
//	game->addPlayer(p1);
//	game->addPlayer(p1);
//	game->addPlayer(p1);
//	game->addPlayer(p1);
//	game->addPlayer(p1);
	game->addPlayer(p2);
//	game->addPlayer(p3);
//	game->addPlayer(p4);
	game->addPlayer(p5);
//	game->addPlayer(this);
	game->newGame(Tile::BaseGame, &tileFactory);

	new std::thread( [this]() {
		while (!game->isFinished())
		{
//			Util::sleep(150);
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

void MainWindow::newGame(int player, const Game * game)
{
	boardUi->newGame(player, game);

	if (isSetUp)
		return;
	isSetUp = true;

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

	ui->remainingTiles->setUp(game, &imgFactory);
	connect(this, SIGNAL(updateNeeded()), ui->remainingTiles, SLOT(updateView()));


	QSettings settings;
	settings.beginGroup("games");
	int size = settings.beginReadArray("game");
	settings.endArray();

	settings.beginWriteArray("game");
	settings.setArrayIndex(size);

	settings.setValue("appRevision", APP_REVISION_STR);
	settings.setValue("qtCompileVersion", QT_VERSION_STR);
	settings.setValue("qtRuntimeVersionn", qVersion());
	settings.beginWriteArray("players");
	auto const & players = game->getPlayers();
	for (size_t i = 0; i < players.size(); ++i)
	{
		settings.setArrayIndex((int)i);
		settings.setValue("type", players[i]->getTypeName());
	}
	settings.endArray();

	settings.endArray();
	settings.endGroup();
}

void MainWindow::playerMoved(int player, const Tile * tile, const MoveHistoryEntry & move)
{
	emit updateNeeded();
	boardUi->playerMoved(player, tile, move);

	QSettings settings;
	settings.beginGroup("games");
	int size = settings.beginReadArray("game");
	settings.endArray(); //game

	settings.beginWriteArray("game");
	settings.setArrayIndex(size-1);

	int moveSize = settings.beginReadArray("moves");
	settings.endArray(); //moves

	auto const & history = game->getMoveHistory();
	settings.beginWriteArray("moves");
	for (size_t i = moveSize; i < history.size(); ++i)
	{
		settings.setArrayIndex((int)i);
		MoveHistoryEntry const & m = history[i];
		settings.setValue("tile", m.tile);
		settings.setValue("x", m.move.tileMove.x);
		settings.setValue("y", m.move.tileMove.x);
		settings.setValue("orientation", m.move.tileMove.orientation);
		settings.setValue("meeple", m.move.meepleMove.nodeIndex);
	}
	settings.endArray(); //moves

	settings.endArray(); //game
	settings.endGroup(); //games
}

TileMove MainWindow::getTileMove(int player, const Tile * tile, const MoveHistoryEntry & move, const TileMovesType & placements)
{
	return boardUi->getTileMove(player, tile, move, placements);
}

MeepleMove MainWindow::getMeepleMove(int player, const Tile * tile, const MoveHistoryEntry & move, const MeepleMovesType & possible)
{
	return boardUi->getMeepleMove(player, tile, move, possible);
}

void MainWindow::endGame()
{
	isSetUp = false;
	emit updateNeeded();
	boardUi->endGame();


	QSettings settings;
	settings.beginGroup("games");
	int size = settings.beginReadArray("game");
	settings.endArray();

	settings.beginWriteArray("game");
	settings.setArrayIndex(size-1);

	settings.beginWriteArray("result");
	int playerCount = game->getPlayerCount();
	for (int i = 0; i < playerCount; ++i)
	{
		settings.setArrayIndex(i);
		settings.setValue("score", game->getPlayerScore(i));
	}
	settings.endArray();

	settings.endArray();
	settings.endGroup();
}

void MainWindow::closeEvent(QCloseEvent * event)
{
	QSettings settings;
	settings.beginGroup("mainWindow");
	settings.setValue("geometry", saveGeometry());
	settings.setValue("windowState", saveState());
	QMainWindow::closeEvent(event);

	settings.endGroup();
}

void MainWindow::readSettings()
{
	QSettings settings;
	settings.beginGroup("mainWindow");
	restoreGeometry(settings.value("geometry").toByteArray());
	restoreState(settings.value("windowState").toByteArray());

	quint64 id = 0;
	if (settings.contains("appId"))
	{
		id = settings.value("appId").value<quint64>();
	}
	else
	{
		std::uniform_int_distribution<quint64> distribution;
		std::default_random_engine generator(std::chrono::duration_cast<std::chrono::nanoseconds>(std::chrono::high_resolution_clock::now().time_since_epoch()).count());
		for (int i = 0; i < 5000; ++i)
			distribution(generator);
		for (quint64 i = 0, end = distribution(generator) % 1000000; i < end; ++i)
			distribution(generator);
		for (uint i = 0; i <= sizeof(id) * 8; ++i)
			id = (id << 8) ^ distribution(generator);
		settings.setValue("appId", id);
	}

	settings.endGroup();
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

void MainWindow::on_actionRandom_Tiles_toggled(bool checked)
{
	if (checked)
		game->setNextTileProvider(&rntp);
}

void MainWindow::on_actionChoose_Tiles_toggled(bool checked)
{
	if (checked)
		game->setNextTileProvider(ui->remainingTiles);
}
