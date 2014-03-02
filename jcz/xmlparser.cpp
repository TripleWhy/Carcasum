#include "xmlparser.h"

#include <QFile>
#include <QXmlStreamReader>
#include <QXmlStreamAttributes>
#include <QHash>
#include <QTransform>

QList<jcz::XmlParser::XMLTile> jcz::XmlParser::readTileDefinitions(jcz::Expansion expansion)
{
	QList<XMLTile> tiles;

	QString file = QString(":/jcz/tile-definitions/%1.xml").arg(Expansions::getName(expansion).toLower());
	QFile f(file);
	f.open(QIODevice::ReadOnly);
	QXmlStreamReader xml(&f);

	xml.readNextStartElement();
	if (xml.name() != "pack")
		qWarning() << "unknown element:" << xml.name();

	else
	{
		int index = 0;
		while (xml.readNextStartElement())
		{
			Q_ASSERT(xml.tokenType() == QXmlStreamReader::StartElement);
			if (xml.name() == "tile")
			{
				XMLTile tile;
				tile.expansion = expansion;
				tile.index = index++;
				readTile(xml, tile);

				tiles.append(tile);
			}
			else
				qWarning() << "unknown element:" << xml.name();
		}
		if (xml.hasError())
		{
			qWarning() << xml.errorString();
		}
	}

	f.close();

	return tiles;
}

jcz::Location initFromDirList(QStringList const & sides)
{
	jcz::Location loc = jcz::Location::null;
	for (int i = 0; i < sides.length(); i++) {
		jcz::Location l = jcz::Location::valueOf(sides[i]);
//        assert !(piece instanceof Farm ^ l.isFarmLocation()) : String.format("Invalid location %s kind for tile %s", l, tile.getId());
//        assert l.intersect(loc) == null;
		loc = (loc.isNull()) ? l : loc.unite(l);
	}
	//logger.debug(tile.getId() + "/" + piece.getClass().getSimpleName() + "/"  + loc);
//	piece.setTile(tile);
//	piece.location = loc;
//	features.add(piece);
	return loc;
}

void jcz::XmlParser::readTile(QXmlStreamReader & xml, XMLTile & tile)
{
	qDebug() << xml.tokenString() << xml.name();
	bool ok = false;
	int count = xml.attributes().value("count").toInt(&ok);
	if (!ok || count <= 0)
		return;
	tile.count = count;
	tile.id = xml.attributes().value("id").toString();

	while (xml.readNextStartElement())
	{
		qDebug() << xml.tokenString() << xml.name();

		QString name = xml.name().toString();
		if (name == "wagon-move")
		{
			//TODO
		}
		else if (name == "position")
		{
			//TODO
		}
		else
		{
			XMLFeature feature;
			feature.name = name;

			for (QXmlStreamAttribute const & a : xml.attributes())
				feature.properties[a.name().toString()] = a.value().toString();

			if (name == "cloister")
				feature.location = Location::CLOISTER;
			else if (name == "tower")
				feature.location = Location::TOWER;
			else
			{
				xml.readNext();
				qDebug() << xml.tokenString() << xml.name() << xml.text();

				QStringList const & split = xml.text().toString().split(' ', QString::SkipEmptyParts);
				feature.location = initFromDirList(split);
			}

			Q_ASSERT(!tile.features.contains(feature.location));
			tile.features.insert(feature.location, feature);
		}

		xml.skipCurrentElement();
		qDebug();
	}
	qDebug();
}

void jcz::XmlParser::readPoints(QString const & file, QList<XMLTile> & tiles)
{
	QFile f(file);
	f.open(QIODevice::ReadOnly);
	QXmlStreamReader xml(&f);

	xml.readNextStartElement();
	if (xml.name() != "points")
		qWarning() << "unknown element:" << xml.name();
	else
	{
		while (xml.readNextStartElement())
		{
			Q_ASSERT(xml.tokenType() == QXmlStreamReader::StartElement);
			if (xml.name() == "point")
			{
				readPoint(xml, tiles);
			}
			else
				qWarning() << "unknown element:" << xml.name();
		}
		if (xml.hasError())
		{
			qWarning() << xml.errorString();
			Q_ASSERT(false);
		}
	}

	f.close();
}

void jcz::XmlParser::readPoint(QXmlStreamReader & xml, QList<XMLTile> & tiles)
{
	bool ok1, ok2;
	QString featureName = xml.attributes().value("feature").toString().toLower();
	int cx = xml.attributes().value("cx").toInt(&ok1);
	int cy = xml.attributes().value("cy").toInt(&ok2);
	Location baseLocation = Location::valueOf(xml.attributes().value("baseLocation").toString());

	if (featureName.isEmpty() || !ok1 || !ok2)
	{
		qWarning() << "bad point definition";
		Q_ASSERT(false);
		return;
	}

	qDebug() << featureName << cx << cy << xml.attributes().value("baseLocation");

	while (xml.readNextStartElement())
	{
		if (xml.name() != "apply")
		{
			//TODO rotate(..., 500, 500)
			qWarning() << "bad point apply definition";
//			Q_ASSERT(false);
//			return;

			xml.skipCurrentElement();
			continue;
		}

		bool allRotations = (xml.attributes().value("allRotations") == "1");

		xml.readNext();
		qDebug() << "apply" << xml.text();

		QStringList const & split = xml.text().toString().split(' ', QString::SkipEmptyParts);
		if (split.length() != 2)
		{
			qWarning() << "bad point apply location definition";
			Q_ASSERT(false);
			return;
		}

		QString const & fullTileName = split.at(0);
		QString const & locationName = split.at(1);

		QList<Location> locations;
		{
			Location location = Location::valueOf(locationName);
			locations.append(location);
			if (allRotations)
				for (uchar i = 1; i < 4; ++i)
					locations.append(location.rotateCW(i));
		}

		bool allTiles = true;
		QString expansionName;
		QString tileName;
		if (fullTileName != "*")
		{
			allTiles = false;

			if (fullTileName.indexOf('.') != 2)
			{
				qWarning() << "bad point apply location definition";
				Q_ASSERT(false);
				return;
			}
			expansionName = fullTileName.left(2);
			tileName = fullTileName.mid(3);
		}


		QPoint point(cx, cy);
		Q_ASSERT(!point.isNull());
		for (int i = 0; i < locations.length(); ++i)
		{
			Location const & l = locations.at(i);
			QTransform t;
			t.translate(500, 500);
			t.rotate(-i * 90);

			signed char r = l.getRotationOf(baseLocation);
			if (r > 0)
				t.rotate(-r * 90);

			t.translate(-500, -500);
			QPoint rotatedPoint = t.map(point);

			for (XMLTile & tile : tiles)
			{
				if (!allTiles)
				{
					if (tile.id != tileName)
						continue;
					if (Expansions::getCode(tile.expansion) != expansionName)
						continue;
				}

				if (!tile.features.contains(l))
					continue;
				XMLFeature & feature = tile.features[l];
				if (feature.name != featureName)
					continue;

				feature.point = rotatedPoint;
			}
		}

		xml.skipCurrentElement();
	}
	qDebug();
}
