#include "mainwindow.h"
#include <QApplication>

#include "static.h"
#include "core/game.h"
#include "gui/boardgraphicsscene.h"
#include "jcz/xmlparser.h"

int main(int argc, char *argv[])
{
	QApplication a(argc, argv);
	QCoreApplication::setOrganizationName(APP_ORGANIZATION);
	QCoreApplication::setApplicationName(APP_NAME);

	qDebug() << "Qt build version:  " << QT_VERSION_STR;
	qDebug() << "Qt runtime version:" << qVersion();
	qDebug() << "Git revision:" << APP_REVISION_STR;

	MainWindow w;
	w.show();

	return a.exec();
	return 0;
}
