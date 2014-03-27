#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "core/game.h"
#include "core/player.h"
#include "boardgraphicsscene.h"

#include <QMainWindow>

namespace Ui {
class MainWindow;
}

class PlayerInfoView;

#include <QTimer>
class MainWindow : public QMainWindow, public Player
{
	Q_OBJECT
private:
	Game * game;
	QTimer * timer;
	BoardGraphicsScene * boardUi;
	std::vector<PlayerInfoView *> playerInfos;
	
	jcz::TileFactory tileFactory;
	TileImageFactory imgFactory = TileImageFactory(&tileFactory);

public:
	explicit MainWindow(QWidget *parent = 0);
	~MainWindow();
	
	virtual void newGame(int player, const Game * game);
	virtual void playerMoved(int player, Tile const * tile, MoveHistoryEntry const & move);
	virtual TileMove getTileMove(int player, Tile const * tile, MoveHistoryEntry const & move, TileMovesType const & placements);
	virtual MeepleMove getMeepleMove(int player, Tile const * tile, MoveHistoryEntry const & move, MeepleMovesType const & possible);
	virtual void endGame();

signals:
	void updateNeeded();

private slots:
	void timeout();
	void recenter(QRectF rect);

private:
	Ui::MainWindow *ui;
};

#endif // MAINWINDOW_H
