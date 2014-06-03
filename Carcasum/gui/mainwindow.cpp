#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "playerinfoview.h"
#include "jcz/tilefactory.h"
#include "player/randomplayer.h"
#include "renderoptionsdialog.h"
#include <QSettings>
#include <QFileDialog>
#include <QStandardPaths>
#include <QDateTime>
#include <QMessageBox>

void MainWindow::GameThread::undo(int steps)
{
	undoSteps = steps;
	interrupt();
}

void MainWindow::GameThread::run()
{
	clearInterrupt();
	setTerminationEnabled(true);

	for (; undoSteps > 0; --undoSteps)
		g->undo();

	while (!g->isFinished())
	{
		msleep(PLAYER_GAP_TIMEOUT);
		g->step();

		if (isInterrupted() && undoSteps == 0)
			break;
		else
			clearInterrupt();

		for (; undoSteps > 0; --undoSteps)
		{
			while (g->undo())
			{}
		}
	}
}


MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
	ui->setupUi(this);
#if !MAINWINDOW_ENABLE_UNDO
	delete ui->actionUndo;
#endif
	ui->optionsWidget->setVisible(ui->optionsCheckBox->isChecked());
	setWindowTitle(APP_NAME);

	playerSelector = new PlayerSelector(&tileFactory, this);

	for (int i = 1; i <= MAX_PLAYERS; ++i)
	{
		QLabel * numberLabel = new QLabel(QString::number(i));
		ui->ngPlayerLayout->addWidget(numberLabel, i, 0);

		QComboBox * colorBox = new QComboBox();
		for (uint j = 0; j < colors.size(); ++j)
		{
			colorBox->addItem(colorNames[j]);

			QPixmap px(32, 32);
			QPainter painter(&px);
			painter.fillRect(px.rect(), colors[j]);
			painter.drawRect(0, 0, px.width() - 1, px.height() - 1);
			colorBox->setItemIcon(j, QIcon(px));
		}
		colorBox->setCurrentIndex(i-1);
		ui->ngPlayerLayout->addWidget(colorBox, i, 1);
		connect(colorBox, SIGNAL(currentIndexChanged(int)), this, SLOT(colorBoxChanged(int)));

		QComboBox * typeBox = new QComboBox();
		typeBox->addItems(QStringList{tr("", "player selection: no player"), tr("Player"), tr("Computer")});
		ui->ngPlayerLayout->addWidget(typeBox, i, 2);
		connect(typeBox, SIGNAL(currentIndexChanged(int)), this, SLOT(typeBoxChanged(int)));

		QLineEdit * nameEdit = new QLineEdit();
		ui->ngPlayerLayout->addWidget(nameEdit, i, 3);

		ngPlayerEdits[i-1] = NgPlayerEdit{colorBox, typeBox, nameEdit};
	}

	auto actionGroup = new QActionGroup(this);
	actionGroup->addAction(ui->actionRandom_Tiles);
	actionGroup->addAction(ui->actionChoose_Tiles);

	boardUi = new BoardGraphicsScene(&tileFactory, &imgFactory, ui->boardView);
	game = new Game(this, true);
	game->addView(this);
	gameThread = new GameThread(game, this);

	boardUi->setGame(game);
	ui->boardView->setScene(boardUi);

	connect(boardUi, SIGNAL(sceneRectChanged(QRectF)), this, SLOT(recenter(QRectF)));
	connect(this, SIGNAL(gameEvent(QString)), this, SLOT(displayGameEvent(QString)));
	connect(this, SIGNAL(gameEventPop()), this, SLOT(displayGameEventPop()));

	readSettings();

	game->addWatchingPlayer(this);
}

MainWindow::~MainWindow()
{
	delete ui;
	for (Player *& p : selectedPlayers)
	{
		if (p != this)
			delete p;
		p = 0;
	}
}

void MainWindow::newGame(int player, const Game * game)
{
	gameStartTimestamp = QDateTime::currentMSecsSinceEpoch();

	if (player < 0)
	{
		qDebug("New game started with players:");
		for (Player const * p : game->getPlayers())
			qDebug() << p->getTypeName();
		qDebug();
	}

	boardUi->newGame(player, game);

	if (isSetUp)
		return;
	isSetUp = true;

	qDeleteAll(playerInfos);
	playerInfos.clear();
	for (uint i = 0; i < game->getPlayerCount(); ++i)
	{
		PlayerInfoView * pi = new PlayerInfoView(i, game, &imgFactory);
		ui->playerInfoLayout->insertWidget(i, pi);
		connect(this, SIGNAL(updateNeeded()), pi, SLOT(updateView()));
		connect(this, SIGNAL(tileDrawn(int,int)), pi, SLOT(displayTile(int,int)));
		playerInfos.push_back(pi);
	}

	emit gameEvent(tr("New game started"));

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
	settings.setValue("timestamp", gameStartTimestamp);
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

	if (move.move.tileMove.isNull())
		emit gameEvent(tr("A non-suitable tile was drawn and discarded."));
	else
		emit gameEvent(tr("Player %1 moved.").arg(player+1));

//	QSettings settings;
//	settings.beginGroup("games");
//	int size = settings.beginReadArray("game");
//	settings.endArray(); //game

//	settings.beginWriteArray("game");
//	settings.setArrayIndex(size-1);

//	int moveSize = settings.beginReadArray("moves");
//	settings.endArray(); //moves

//	auto const & history = game->getMoveHistory();
//	settings.beginWriteArray("moves");
//	for (size_t i = moveSize; i < history.size(); ++i)
//	{
//		settings.setArrayIndex((int)i);
//		MoveHistoryEntry const & m = history[i];
//		settings.setValue("tile", m.tile);
//		settings.setValue("x", m.move.tileMove.x);
//		settings.setValue("y", m.move.tileMove.y);
//		settings.setValue("orientation", m.move.tileMove.orientation);
//		settings.setValue("meeple", m.move.meepleMove.nodeIndex);
//	}
//	settings.endArray(); //moves

//	settings.endArray(); //game
//	settings.endGroup(); //games

	QString const & dataLoc = QStandardPaths::writableLocation(QStandardPaths::DataLocation);
	QDir dir = QDir(dataLoc).absoluteFilePath("games");
	if (dir.mkpath(".") && gameStartTimestamp != -1)
		game->storeToFile(dir.absoluteFilePath(QString::number(gameStartTimestamp)));
}

void MainWindow::undoneMove(const MoveHistoryEntry & move)
{
	emit updateNeeded();
	boardUi->undoneMove(move);

	emit gameEventPop();

	QString const & dataLoc = QStandardPaths::writableLocation(QStandardPaths::DataLocation);
	QDir dir = QDir(dataLoc).absoluteFilePath("games");
	if (dir.mkpath(".") && gameStartTimestamp != -1)
		game->storeToFile(dir.absoluteFilePath(QString::number(gameStartTimestamp)));
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

	emit gameEvent(tr("Game ended."));

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

Player *MainWindow::clone() const
{
	return 0;
}

void MainWindow::nodeScored(const Node * n, const int score, const Game * game)
{
	QStringList players;
	uchar meepleCount = n->getMaxMeeples();
	{
		int player = 0;
		for (uchar const * m = n->getMeeples(), * end = m + game->getPlayerCount(); m < end; ++m, ++player)
		{
			if (*m == meepleCount)
				players.append(QString::number(player+1));
		}
	}
	QString name;
	switch (n->getTerrain())
	{
		case Field:
			name = tr("Field");
			break;
		case City:
			name = tr("City");
			break;
		case Road:
			name = tr("Road");
			break;
		case Cloister:
			name = tr("Cloister");
			break;
		case None:
		default:
			name = tr("[Unknown]");
			break;
	}

	emit gameEvent(tr("%1 scored %n point(s) for players: %2", "", score).arg(name).arg(players.join(", ")));
}

void MainWindow::nodeUnscored(const Node * /*n*/, const int /*score*/, const Game * /*game*/)
{
}

int MainWindow::nextTile(const Game * game)
{
	int result;
	if (randomTiles)
		result = rntp.nextTile(game);
	else
		result = ui->remainingTiles->nextTile(game);

	int player = game->getNextPlayer();
	TileTypeType tileType = game->getTiles()[result]->tileType;
	emit tileDrawn(player, tileType);

	return result;
}

void MainWindow::closeEvent(QCloseEvent * event)
{
	requestEndGame();
	boardUi->quit();

	QSettings settings;
	settings.beginGroup("mainWindow");
	settings.setValue("geometry", saveGeometry());
	settings.setValue("windowState", saveState());
	settings.setValue("splitterState", ui->splitter->saveState());
	settings.setValue("splitter2State", ui->splitter2->saveState());
	settings.endGroup();

	forceEndGame();
	QMainWindow::closeEvent(event);
}

#if MAINWINDOW_GAME_ON_STARTUP
#include "jcz/jczplayer.h"
#include "player/simpleplayer.h"
#include "player/simpleplayer2.h"
bool MainWindow::event(QEvent * event)
{
	if (event->type() == QEvent::UpdateRequest && !gameThread->isRunning() && !gameThread->isFinished())
	{
		std::vector<MoveHistoryEntry> history;

//		ui->actionChoose_Tiles->setChecked(true);
//		history = Game::loadFromFile("../../Carcasum/states/state01");
//		if (history.size() > 0)
//		{
//			//Remove last move.
//			auto * tp = new HistoryProvider(game->getNextTileProvider(), history, history.size() - 1);
//			game->setNextTileProvider(tp);
//			history.pop_back();
//		}

//		game->addPlayer(new jcz::JCZPlayer(&tileFactory));
		game->addPlayer(this);
		game->addPlayer(this);
//		game->addPlayer(new SimplePlayer());
//		game->addPlayer(new SimplePlayer2());
//		game->addPlayer(&RandomPlayer::instance);
//		game->addPlayer(&RandomPlayer::instance);

		game->newGame(Tile::BaseGame, &tileFactory, history, true);
		ui->stackedWidget->setCurrentWidget(ui->gameDisplayPage);
		gameThread->start();
	}
	return QMainWindow::event(event);
}
#endif

void MainWindow::readSettings()
{
	QSettings settings;
	settings.beginGroup("mainWindow");
	restoreGeometry(settings.value("geometry").toByteArray());
	restoreState(settings.value("windowState").toByteArray());
	ui->splitter->restoreState(settings.value("splitterState").toByteArray());
	ui->splitter2->restoreState(settings.value("splitter2State").toByteArray());

	quint64 id = 0;
	if (settings.contains("appId"))
	{
		id = settings.value("appId").value<quint64>();
	}
	else
	{
		std::uniform_int_distribution<quint64> distribution;
		std::default_random_engine generator((std::default_random_engine::result_type)std::chrono::duration_cast<std::chrono::nanoseconds>(std::chrono::high_resolution_clock::now().time_since_epoch()).count());
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

void MainWindow::requestEndGame()
{
	gameThread->interrupt();
}

void MainWindow::forceEndGame()
{
	requestEndGame();
	gameThread->wait(1000);
	gameThread->terminate();
	gameThread->wait(1000);
}

void MainWindow::displayGameEvent(const QString & msg)
{
	ui->eventList->addItem(QString("%1: %2").arg(QTime::currentTime().toString(Qt::TextDate)).arg(msg));
	ui->eventList->scrollToBottom();
}

void MainWindow::displayGameEventPop()
{
	//TODO if something was scored, this needs to remove more than one entry.
	delete ui->eventList->takeItem(ui->eventList->count() - 1);
	ui->eventList->scrollToBottom();
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

void MainWindow::colorBoxChanged(int index)
{
	QObject * snd = sender();

	bool colorUsed[MAX_PLAYERS];
	for (int i = 0; i < MAX_PLAYERS; ++i)
		colorUsed[i] = false;

	for (NgPlayerEdit const & pe : ngPlayerEdits)
	{
		QComboBox * cb = pe.colorBox;
		colorUsed[cb->currentIndex()] = true;
	}
	for (NgPlayerEdit const & pe : ngPlayerEdits)
	{
		QComboBox * cb = pe.colorBox;
		if (cb == snd)
			continue;
		if (cb->currentIndex() == index)
		{
			for (int i = 0; i < MAX_PLAYERS; ++i)
			{
				if (!colorUsed[i])
				{
					colorUsed[i] = true;
					cb->setCurrentIndex(i);
					break;
				}
			}
		}
	}
}

void MainWindow::typeBoxChanged(int index)
{
	QComboBox * snd = static_cast<QComboBox *>(sender());
	int playerIndex = 0;
	for (NgPlayerEdit const & pe : ngPlayerEdits)
	{
		QComboBox * cb = pe.typeBox;
		if (cb == snd)
			break;
		++playerIndex;
	}

	Player *& p = selectedPlayers[playerIndex];
	if (index != 3)
	{
		if (p != this)
			delete p;
		snd->removeItem(3);
	}
	switch (index)
	{
		case 0:
			p = 0;
			break;
		case 1:
			p = this;
			break;
		case 2:
		{
			if (playerSelector->exec() == QDialog::Accepted)
			{
				p = playerSelector->createPlayer();
				snd->addItem(playerSelector->playerDisplayName());
				snd->setCurrentIndex(3);
			}
			else
			{
				p = 0;
				snd->setCurrentIndex(0);
			}
		}
	}
}

void MainWindow::on_actionRandom_Tiles_toggled(bool checked)
{
	if (checked)
		randomTiles = true;
}

void MainWindow::on_actionChoose_Tiles_toggled(bool checked)
{
	if (checked)
		randomTiles = false;
}

void MainWindow::on_buttonBox_accepted()
{
	requestEndGame();

	std::vector<MoveHistoryEntry> history;
	if (ui->optionsCheckBox->isChecked())
	{
		QString path = ui->boardFileEdit->text();
		QFileInfo fi(path);
		if (fi.exists())
			history = Game::loadFromFile(path);
	}

	forceEndGame();

	game->clearPlayers();
	for (Player * p : selectedPlayers)
	{
		if (p != 0)
		{
			game->addPlayer(p);
		}
	}
	for (uint i = 0, p = 0; i < ngPlayerEdits.size(); ++i)
	{
		NgPlayerEdit const & pe = ngPlayerEdits[i];
		if (pe.typeBox->currentIndex() == 0)
			continue;

		QColor color = colors[pe.colorBox->currentIndex()];
		imgFactory.setPlayerColor(p, color);

		QString name = pe.nameEdit->text();
		if (name.isEmpty())
		{
			if (pe.typeBox->currentIndex() == 3)
				name = pe.typeBox->currentText();
			else
				name = tr("Player %1").arg(p+1);
		}
		imgFactory.setPlayerName(p, name);

		++p;
	}

	if (game->getPlayerCount() < 1)
		return;
#if DISPLAY_WHILE_LOADING
	ui->stackedWidget->setCurrentWidget(ui->gameDisplayPage);
#endif
	game->newGame(Tile::BaseGame, &tileFactory, history, true);

	ui->stackedWidget->setCurrentWidget(ui->gameDisplayPage);

	QSettings settings;
	if (settings.value("firstStart", true).toBool())
	{
		on_actionControls_triggered();
		settings.setValue("firstStart", false);
	}

	gameThread->start();
}

void MainWindow::on_actionNew_Game_triggered()
{
	requestEndGame();
	ui->stackedWidget->setCurrentWidget(ui->newGamePage);
}

void MainWindow::on_actionStore_board_triggered()
{
	auto history = game->getMoveHistory();
	QString path = QFileDialog::getSaveFileName(this, tr("Save Board"));
	if (!path.isEmpty())
		Game::storeToFile(path, history);
}

void MainWindow::on_boardFileButton_clicked()
{
	QString path = QFileDialog::getOpenFileName(this, tr("Board File"));
	ui->boardFileEdit->setText(path);
}

void MainWindow::on_actionControls_triggered()
{
	QMessageBox::information(this, tr("Controls"),
							 tr("Game Controls\n\n"
							 "Place tile: left mouse button\n"
							 "Rotate tile: right mouse button\n\n"
							 "Place meeple: left mouse button\n"
							 "Place no meeple: right mouse button\n\n"
							 "Move board: middle mouse button\n"
							 "Zoom: mouse wheel"));
}

void MainWindow::on_actionUndo_triggered()
{
#if MAINWINDOW_ENABLE_UNDO
	gameThread->undo();
#endif
}

void MainWindow::on_actionRender_to_file_triggered()
{
	QString path = QFileDialog::getSaveFileName(this, tr("Render Board"));
	if (path.isEmpty())
		return;

	RenderOptionsDialog renderOptions(this);
	if (renderOptions.exec() != QDialog::Accepted)
		return;

	renderBoard(
	            game->getMoveHistory(),
	            path,
	            renderOptions.getRemoveLast(),
	            renderOptions.getRenderOpenTiles(),
	            renderOptions.getRenderFrames(),
	            renderOptions.getRenderPlayers(),
	            renderOptions.getRenderNextTile()
	            );
}

void MainWindow::renderBoard(QString inFile, QString outFile, int removeLast, bool renderOpenTiles, bool renderFrames, bool renderPlayers, bool renderNextTile)
{
	auto && history = Game::loadFromFile(inFile);
	if (history.size() == 0)
		return;
	return renderBoard(history, outFile, removeLast, renderOpenTiles, renderFrames, renderPlayers, renderNextTile);
}

void MainWindow::renderBoard(std::vector<MoveHistoryEntry> history, QString outFile, int removeLast, bool renderOpenTiles, bool renderFrames, bool renderPlayers, bool renderNextTile)
{
	//TODO correct backgrounds

	jcz::TileFactory tileFactory(false);
	TileImageFactory imgFactory(&tileFactory);

	QImage image;
	QPainter * painter = 0;

	RandomNextTileProvider rntp;
	HistoryProvider hp(&rntp, history, history.size() - removeLast);
	Game g(&hp, false);
	g.addPlayer(&RandomPlayer::instance);
	g.addPlayer(&RandomPlayer::instance);

	BoardGraphicsScene bgs(&tileFactory, &imgFactory);
	bgs.setRenderOpenTiles(renderOpenTiles);
	bgs.setRenderFrames(renderFrames);
	bgs.setGame(&g);
	g.addWatchingPlayer(&bgs);

	history.erase(history.end() - removeLast, history.end());
	g.newGame(Tile::BaseGame, &tileFactory, history, true);

	bgs.clearSelection();
	bgs.setSceneRect(bgs.itemsBoundingRect());

	int const spacing = 50;
	int constexpr pivScale = 4;
	QSize sceneSize = bgs.sceneRect().size().toSize();
	QSize size = sceneSize;
	QPoint offset(0, 0);
	if (renderPlayers)
	{
		QLabel remainingLabel(QString("%1 tiles left").arg(g.getTileCount() - 1));
		remainingLabel.updateGeometry();
		int nextTileType = g.getTile(hp.nextTile(&g))->tileType;

		QSize pivSize;
		for (uint p = 0; p < g.getPlayerCount(); ++p)
		{
			PlayerInfoView piv(p, &g, &imgFactory);
			piv.updateView();
			if (renderNextTile)
				piv.displayTile(g.getNextPlayer(), nextTileType);
			piv.updateGeometry();

			if (p == 0)
			{
				pivSize = piv.size() * pivScale;
				pivSize.rheight() *= g.getPlayerCount();

				QSize remSize = remainingLabel.sizeHint() * pivScale;

				size.rwidth() += pivSize.width() + spacing;
				size.rheight() = qMax(size.height(), pivSize.height() + spacing + remSize.height());

				image = QImage(size, QImage::Format_ARGB32);
				image.fill(Qt::transparent);
				painter = new QPainter(&image);
				painter->setRenderHints(QPainter::Antialiasing | QPainter::TextAntialiasing | QPainter::SmoothPixmapTransform | QPainter::HighQualityAntialiasing);

				painter->scale(pivScale, pivScale);
			}

			piv.render(painter, offset);

			offset.ry() += piv.size().height();
		}
		offset.setX(0);
		offset.setY((pivSize.height() + spacing) / pivScale);

		remainingLabel.render(painter, offset);

		offset.setX(pivSize.width() + spacing);
		offset.setY(0);
		painter->resetTransform();
	}
	else
	{
		image = QImage(size, QImage::Format_ARGB32);
		image.fill(Qt::transparent);
		painter = new QPainter(&image);
		painter->setRenderHints(QPainter::Antialiasing | QPainter::TextAntialiasing | QPainter::SmoothPixmapTransform | QPainter::HighQualityAntialiasing);
	}

	bgs.render(painter, QRectF(offset, size));

	image.save(outFile);

	delete painter;
}

void MainWindow::renderBoardCompleteGame(std::vector<MoveHistoryEntry> history, QString outDir, bool renderOpenTiles, bool renderFrames, bool renderPlayers, bool renderNextTile, int playerCount)
{
	//TODO correct backgrounds

	QDir(outDir).mkpath(".");

	jcz::TileFactory tileFactory(false);
	TileImageFactory imgFactory(&tileFactory);

	QImage image;
	QPainter * painter = 0;

	RandomNextTileProvider rntp;
	HistoryProvider hp(&rntp, history, 0);
	Game g(&hp, false);

	for (int i = 0; i < playerCount; ++i)
		g.addPlayer(&RandomPlayer::instance);

	BoardGraphicsScene bgs(&tileFactory, &imgFactory);
	bgs.setRenderOpenTiles(renderOpenTiles);
	bgs.setRenderFrames(renderFrames);
	bgs.setGame(&g);
	g.addWatchingPlayer(&bgs);

	g.newGame(Tile::BaseGame, &tileFactory, history, true);

	bgs.clearSelection();
	QRectF sceneRect = bgs.itemsBoundingRect();

	int const spacing = 50;
	int constexpr pivScale = 4;
	QSize sceneSize = sceneRect.size().toSize();
	QSize size = sceneSize;
	if (renderPlayers)
	{
		QLabel remainingLabel(QString("%1 tiles left").arg(g.getTileCount() - 1));
		remainingLabel.updateGeometry();
		int nextTileType = g.getTile(hp.nextTile(&g))->tileType;

		QSize pivSize;
		{
			PlayerInfoView piv(0, &g, &imgFactory);
			piv.updateView();
			if (renderNextTile)
				piv.displayTile(g.getNextPlayer(), nextTileType);
			piv.updateGeometry();

			pivSize = piv.size() * pivScale;
			pivSize.rheight() *= g.getPlayerCount();

			QSize remSize = remainingLabel.sizeHint() * pivScale;

			size.rwidth() += pivSize.width() + spacing;
			size.rheight() = qMax(size.height(), pivSize.height() + spacing + remSize.height());
		}
	}



	std::vector<MoveHistoryEntry> applyHistory;
	for (int i = -1; i < (int)history.size(); ++i)
	{
		// oh well, this is ugly. If only there was a Game::step(MoveHistoryEntry)...
		if (i >= 0)
			applyHistory.push_back(history[i]);
		g.newGame(Tile::BaseGame, &tileFactory, applyHistory, true);

		delete painter;
		image = QImage(size, QImage::Format_ARGB32);
		image.fill(Qt::transparent);
		painter = new QPainter(&image);
		painter->setRenderHints(QPainter::Antialiasing | QPainter::TextAntialiasing | QPainter::SmoothPixmapTransform | QPainter::HighQualityAntialiasing);

		QPoint offset(0, 0);
		if (renderPlayers)
		{
			QLabel remainingLabel(QString("%1 tiles left").arg(g.getTileCount() - 1));
			remainingLabel.updateGeometry();
			int nextTileType = g.getTile(hp.nextTile(&g))->tileType;

			QSize pivSize;
			painter->scale(pivScale, pivScale);
			for (uint p = 0; p < g.getPlayerCount(); ++p)
			{
				PlayerInfoView piv(p, &g, &imgFactory);
				piv.updateView();
				if (renderNextTile)
					piv.displayTile(g.getNextPlayer(), nextTileType);
				piv.updateGeometry();

				piv.render(painter, offset);

				offset.ry() += piv.size().height();
			}
			offset.setX(0);
			offset.setY((pivSize.height() + spacing) / pivScale);

			remainingLabel.render(painter, offset);

			offset.setX(pivSize.width() + spacing);
			offset.setY(0);
			painter->resetTransform();
		}

		bgs.setSceneRect(sceneRect);
		bgs.render(painter, QRectF(offset, size));

		image.save(QString("%1/%2.png").arg(outDir).arg(i+1, 3, 10, QLatin1Char('0')));
	}

	delete painter;
}
