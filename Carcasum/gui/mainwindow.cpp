#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "playerinfoview.h"
#include "jcz/tilefactory.h"
#include <QSettings>
#include <QFileDialog>

void MainWindow::GameThread::run()
{
	setTerminationEnabled(true);
	while (!isInterruptionRequested() && !g->isFinished())
	{
		msleep(100);
		g->step();
	}
}


MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
	ui->setupUi(this);
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
		typeBox->addItems(QStringList{tr(""), tr("Human"), tr("Computer")});
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
	game = new Game(&rntp, true);
	game->addView(this);
	gameThread = new GameThread(game, this);

	boardUi->setGame(game);
	ui->boardView->setScene(boardUi);

	connect(boardUi, SIGNAL(sceneRectChanged(QRectF)), this, SLOT(recenter(QRectF)));

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
	auto timestamp = QDateTime::currentMSecsSinceEpoch();

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

		QString name = ngPlayerEdits[i].nameEdit->text();
		if (name.isEmpty())
		{
			if (ngPlayerEdits[i].typeBox->currentIndex() == 3)
				name = ngPlayerEdits[i].typeBox->currentText();
			else
				name = tr("Player %1").arg(i+1);
		}
		pi->setPlayerName(name);

		l->insertWidget(i, pi);
		connect(this, SIGNAL(updateNeeded()), pi, SLOT(updateView()));
		playerInfos.push_back(pi);
	}

	logEvent(tr("New game started"));

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
	settings.setValue("timestamp", timestamp);
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

	logEvent(tr("Player %1 moved.").arg(player));

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

	logEvent(tr("Game ended."));

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

	logEvent(tr("%1 scored %n point(s) for players: %2", "", score).arg(name).arg(players.join(", ")));
}

void MainWindow::nodeUnscored(const Node * /*n*/, const int /*score*/, const Game * /*game*/)
{
}

void MainWindow::closeEvent(QCloseEvent * event)
{
	requestEndGame();
	qWarning("MainWindow::closeEvent: If the following line warns about an invalid move, ignore it.");
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

void MainWindow::requestEndGame()
{
	gameThread->requestInterruption();
}

void MainWindow::forceEndGame()
{
	requestEndGame();
	gameThread->wait(1000);
	gameThread->terminate();
	gameThread->wait(1000);
}

void MainWindow::logEvent(QString const & msg)
{
	ui->eventList->addItem(tr("%1: %2").arg(QTime::currentTime().toString(Qt::TextDate)).arg(msg));
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
		game->setNextTileProvider(&rntp);
}

void MainWindow::on_actionChoose_Tiles_toggled(bool checked)
{
	if (checked)
		game->setNextTileProvider(ui->remainingTiles);
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
			game->addPlayer(p);
	}
	for (uint i = 0; i < ngPlayerEdits.size(); ++i)
	{
		NgPlayerEdit const & pe = ngPlayerEdits[i];
		QColor color = colors[pe.colorBox->currentIndex()];
		imgFactory.setPlayerColor(i, color);
	}

	if (game->getPlayerCount() < 1)
		return;
#if DISPLAY_WHILE_LOADING
	ui->stackedWidget->setCurrentWidget(ui->gameDisplayPage);
#endif
	game->newGame(Tile::BaseGame, &tileFactory, history, true);

	ui->stackedWidget->setCurrentWidget(ui->gameDisplayPage);

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
