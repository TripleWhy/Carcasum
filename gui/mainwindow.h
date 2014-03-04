#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "core/game.h"
#include "core/player.h"
#include "boardgraphicsscene.h"

#include <QMainWindow>

namespace Ui {
class MainWindow;
}

#include <QTimer>
class MainWindow : public QMainWindow, public Player
{
	Q_OBJECT
private:
	Game * game;
	QTimer * timer;
	BoardGraphicsScene * boardUi;

public:
	explicit MainWindow(QWidget *parent = 0);
	~MainWindow();

	virtual TileMove getTileMove(Tile const * const tile, QList<Board::TilePlacement> const & placements, Game const * const game);
	virtual MeepleMove getMeepleMove(Tile const * const tile, QVarLengthArray<MeepleMove, NODE_ARRAY_LENGTH> const & possible, Game const * const game);

private slots:
	void timeout();
	void recenter(QRectF rect);

private:
	Ui::MainWindow *ui;
};

#endif // MAINWINDOW_H
