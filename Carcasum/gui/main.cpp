#include "mainwindow.h"
#include <QApplication>

#include "static.h"
#include "core/game.h"
#include "gui/boardgraphicsscene.h"
#include "jcz/xmlparser.h"
#include "gui/downloader.h"
#include <QMessageBox>
#include <QDir>
#include <QStandardPaths>

int main(int argc, char *argv[])
{
	QApplication a(argc, argv);
	QCoreApplication::setOrganizationName(APP_ORGANIZATION);
	QCoreApplication::setApplicationName(APP_NAME);

	qDebug() << "Qt build version:  " << QT_VERSION_STR;
	qDebug() << "Qt runtime version:" << qVersion();
	qDebug() << "Git revision:" << APP_REVISION_STR;
	qDebug() << "QStandardPaths::DataLocation:" << QStandardPaths::writableLocation(QStandardPaths::DataLocation);

	QDir dir = QDir(QCoreApplication::applicationDirPath());
	QFileInfo fi(dir, QString(TileImageFactory::zipFileName()));
	QByteArray data;
	if (!fi.exists())
	{
		QMessageBox::StandardButton btn = QMessageBox::question(0, "Download File?", QString("%1 was not found in the program directory.\nThis file is needed for better looking tiles.\nDo you want me to download it for you now?").arg(TileImageFactory::zipFileName()));
		if (btn == QMessageBox::Yes)
		{
			QString url = "http://jcloisterzone.com/builds/JCloisterZone-2.6.zip";

			Downloader l;
			l.download(url, "Progress...");
			data = l.downloadedData();

			QFile file(fi.absoluteFilePath());
			file.open(QIODevice::WriteOnly);
			file.write(data);
			file.close();
		}
	}

	MainWindow w;
	w.show();

	return a.exec();
	return 0;
}
