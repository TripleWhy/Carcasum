#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "core/game.h"

#include <QMainWindow>

namespace Ui {
class MainWindow;
}

#include <QTimer>
class MainWindow : public QMainWindow
{
	Q_OBJECT
private:
	Game * game;
	QTimer * timer;

public:
	explicit MainWindow(QWidget *parent = 0);
	~MainWindow();

private slots:
	void timeout();

private:
	Ui::MainWindow *ui;
};

#endif // MAINWINDOW_H
