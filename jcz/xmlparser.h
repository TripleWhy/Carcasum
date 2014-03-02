#ifndef XMLPARSER_H
#define XMLPARSER_H

#include "core/tile.h"
#include "location.h"
#include "expansion.h"

#include <QString>
#include <QXmlStreamReader>
#include <QPoint>

namespace jcz
{

class XmlParser
{
public:
	struct XMLFeature
	{
		QString name;
		Location location;
		QMap<QString, QString> properties;
		QPoint point;
	};
	struct XMLTile
	{
		jcz::Expansion expansion = jcz::BASIC;
		int index = 0;
		int count = 0;
		QString id;
		QHash<Location, XMLFeature> features;
	};

public:
	static QList<XMLTile> readTileDefinitions(Expansion expansion);
	static void readTile(QXmlStreamReader & xml, XMLTile & tile);

	static void readPoints(const QString & file, QList<XMLTile> & tiles);
	static void readPoint(QXmlStreamReader & xml, QList<XMLTile> & tiles);
};

}

#endif // XMLPARSER_H
