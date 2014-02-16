#include "mainwindow.h"
#include <QApplication>

#include "core/game.h"
#include "gui/boardui.h"

int main(int argc, char *argv[])
{
	QApplication a(argc, argv);

	MainWindow w;
	w.show();

	return a.exec();
	return 0;
}
