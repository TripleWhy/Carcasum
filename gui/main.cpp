#include "mainwindow.h"
#include <QApplication>

#include "core/game.h"
#include "gui/boardgraphicsscene.h"
#include "jcz/xmlparser.h"

int main(int argc, char *argv[])
{
	QApplication a(argc, argv);

	QList<jcz::XmlParser::XMLTile> && tiles = jcz::XmlParser::readTileDefinitions(jcz::BASIC);
	jcz::XmlParser::readPoints(":/jcz/defaults/points.xml", tiles);
	jcz::XmlParser::readPoints(":/jcz/resources/plugins/classic/tiles/points.xml", tiles);

	for (jcz::XmlParser::XMLTile const & tile : tiles)
	{
		for (jcz::XmlParser::XMLFeature const & feature : tile.features.values())
		{
			if (feature.point.isNull())
				qDebug() << feature.point.isNull() << tile.id << feature.name << feature.location.toString() << feature.point;
			Q_ASSERT(!feature.point.isNull());
		}
	}
	return 0;

	MainWindow w;
	w.show();

	return a.exec();
	return 0;
}
