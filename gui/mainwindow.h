#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "core/game.h"
#include "core/player.h"

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

public:
	explicit MainWindow(QWidget *parent = 0);
	~MainWindow();

	virtual Move getMove(Tile const * const tile, QList<Board::TilePlacement> const & placements, Game const * const game);

private slots:
	void timeout();

private:
	Ui::MainWindow *ui;
};

#endif // MAINWINDOW_H
