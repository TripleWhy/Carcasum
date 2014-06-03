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
#include <QTranslator>
#include <QLibraryInfo>
#include <QSettings>

#if !MAIN_RENDER_STATES
int main(int argc, char *argv[])
{
	QApplication app(argc, argv);
	QSettings::setDefaultFormat(QSettings::IniFormat);
	QCoreApplication::setOrganizationName(APP_ORGANIZATION);
	QCoreApplication::setApplicationName(APP_NAME);

	qDebug() << "Qt build version:  " << QT_VERSION_STR;
	qDebug() << "Qt runtime version:" << qVersion();
	qDebug() << "Git revision:" << APP_REVISION_STR;
	qDebug() << "QStandardPaths::DataLocation:" << QStandardPaths::writableLocation(QStandardPaths::DataLocation);

	QTranslator qtTranslator;
	qtTranslator.load("qt_" + QLocale::system().name(), QLibraryInfo::location(QLibraryInfo::TranslationsPath));
	app.installTranslator(&qtTranslator);

	QTranslator enTranslator;
	enTranslator.load("carcasum_en");
	app.installTranslator(&enTranslator);

	QTranslator appTranslator;
	appTranslator.load("carcasum_" + QLocale::system().name());
	app.installTranslator(&appTranslator);

#ifndef CLASSIC_TILES
	QDir dir = QDir(QCoreApplication::applicationDirPath());
	QFileInfo fi(dir, QString(TileImageFactory::zipFileName()));
	if (!fi.exists())
	{
		QMessageBox::StandardButton btn = QMessageBox::question(0, QApplication::translate("ZipDownload", "Download File?"), QApplication::translate("ZipDownload", "%1 was not found in the program directory.\nThis file is needed for better looking tiles.\nDo you want me to download it for you now?").arg(TileImageFactory::zipFileName()));
		if (btn == QMessageBox::Yes)
		{
			QString url = "http://jcloisterzone.com/builds/JCloisterZone-2.6.zip";

			Downloader l;
			l.download(url, QApplication::translate("ZipDownload", "Progress..."));
			QByteArray data = l.downloadedData();
			if ((l.result() == QDialog::Accepted) && (data.size() > 0))
			{
				QFile file(fi.absoluteFilePath());
				file.open(QIODevice::WriteOnly);
				file.write(data);
				file.close();
			}
		}
	}
#endif

	MainWindow w;
	w.show();

	return app.exec();
	return 0;
}

#else
#include "player/randomplayer.h"
#include "gui/playerinfoview.h"
int main(int argc, char *argv[])
{
	QApplication app(argc, argv);
	QSettings::setDefaultFormat(QSettings::IniFormat);
	QCoreApplication::setOrganizationName(APP_ORGANIZATION);
	QCoreApplication::setApplicationName(APP_NAME);

	qDebug() << "Qt build version:  " << QT_VERSION_STR;
	qDebug() << "Qt runtime version:" << qVersion();
	qDebug() << "Git revision:" << APP_REVISION_STR;
	qDebug() << "QStandardPaths::DataLocation:" << QStandardPaths::writableLocation(QStandardPaths::DataLocation);

	if (false)
	{
		auto history = Game::loadFromFile("game1");
		MainWindow::renderBoardCompleteGame(history, "./game1-render/", false, false, false, false, 3);
		return 0;
	}

	QDir dir("../../Carcasum/states/");
	dir.mkdir("render");
	QString path = dir.filePath("render");

	QStringList filters;
	filters << "state*";

	QStringList const & files = dir.entryList(filters);
	qDebug() << "count:" << files.size();

	for (QString const & s : files)
	{
		QString const & file = dir.filePath(s);
		qDebug() << file;
		MainWindow::renderBoard(file, QString("%1/%2.png").arg(path).arg(s), 1, false, false, true, true);
	}
}
#endif
