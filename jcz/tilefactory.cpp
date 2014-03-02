#include "tilefactory.h"

#include <QFile>

//#include <QDebug>
jcz::TileFactory::~TileFactory()
{
	for (auto it = tileTemplates.constBegin(); it != tileTemplates.constEnd(); ++it)
		qDeleteAll(it.value());
	tileTemplates.clear();
}

QList<Tile *> jcz::TileFactory::createPack(Tile::TileSets tileSets)
{
	QList<Tile *> pack;
	if (tileSets.testFlag(Tile::BaseGame))
		createPack(Tile::BaseGame, pack);
	return pack;
}

void jcz::TileFactory::createPack(Tile::TileSet tileSet, QList<Tile *> & pack)
{
	if (!tileTemplates.contains(tileSet))
		readXMLPack(tileSet);
	for (Tile * t : tileTemplates[tileSet])
	{
		TileMetaData const & data = tileMetaData[t];
		if (data.hasPosition)
			pack.prepend(new Tile(*t));
		else
			pack.append(new Tile(*t));

		for (uint i = 1; i < data.count; ++i)
			pack.append(new Tile(*t));
	}
}

void jcz::TileFactory::readXMLPack(Tile::TileSet set)
{
	QString file;
	switch (set)
	{
		case Tile::BaseGame:
			file = ":/jcz/tile-definitions/basic.xml";
			break;
	}
	readXMLPack(file, set);
}

void jcz::TileFactory::readXMLPack(QString file, Tile::TileSet tileSet)
{
	QFile f(file);
	f.open(QIODevice::ReadOnly);

	QXmlStreamReader xml(&f);

	xml.readNextStartElement();
	if (xml.name() != "pack")
		qWarning() << "unknown element:" << xml.name();

	while (xml.readNextStartElement())
	{
//		qDebug() << xml.tokenString() << xml.name();
		Q_ASSERT(xml.tokenType() == QXmlStreamReader::StartElement);
		if (xml.name() == "tile")
		{
//			for (QXmlStreamAttribute const & a : xml.attributes())
//				qDebug() << "\t" << a.name() << "=" << a.value();
//			qDebug();
			readXMLTile(xml, tileSet);
		}
		else
		{
			qWarning() << "unknown element:" << xml.name();
			xml.skipCurrentElement();
		}
	}
	if (xml.hasError())
	{
		qWarning() << xml.errorString();
	}

	f.close();
}

inline Node * newFieldNode(Tile * tile)
{
	return new Node(Field, tile);
}

inline Node * newCityNode(Tile * tile, uchar open, bool pennant = false)
{
	return new CityNode(tile, open, pennant ? 1 : 0);
}

inline Node * newRoadNode(Tile * tile, uchar open)
{
	return new RoadNode(tile, open);
}

inline Node * newCloisterNode(Tile * tile)
{
	return new Node(Cloister, tile);
}

inline void convertSide(QString const & str, int & side, int & subside)
{
	if (str.at(0) == 'W')
		side = Tile::left;
	else if (str.at(0) == 'N')
		side = Tile::up;
	else if (str.at(0) == 'E')
		side = Tile::right;
	else if (str.at(0) == 'S')
		side = Tile::down;
	else
		side = -1;

	subside = 1;
	if (str.size() > 1)
	{
		if (str.at(1) == 'R')
			subside = 0;
		else if (str.at(1) == 'L')
			subside = 2;
		else
			side = -1;
	}
}

void jcz::TileFactory::readXMLTile(QXmlStreamReader & xml, Tile::TileSet set)
{
	TileMetaData data;
	bool ok = false;
	int count = xml.attributes().value("count").toInt(&ok);
	if (!ok || count <= 0)
		return;
	data.count = count;

	QString id = xml.attributes().value("id").toString();
	int type = tileTemplates[set].size();

	Tile * tile = new Tile(set, type);
	TerrainType (&edges)[4] = tile->edges;
	QList<Node *> nodes;
	Node * edgeConnectors[4][3] = { {0} };
	while (xml.readNextStartElement())
	{
		if (xml.name() == "cloister")
		{
			Node * cloister = newCloisterNode(tile);
			nodes.append(cloister);
		}
		else if (xml.name() == "farm")
		{
			Node * field = newFieldNode(tile);
			xml.readNext();
			for (QString const & str : xml.text().toString().split(' ', QString::SkipEmptyParts))
			{
				int side;
				int subside = 1;

				convertSide(str, side, subside);
				if (side < 0)
				{
					qWarning() << "unknown side:" << str;
					delete field;
					field = 0;
					break;
				}

				edgeConnectors[side][subside] = field;

				if (edges[side] == None)
					edges[side] = Field;
			}

			//TODO
			QString const & city = xml.attributes().value("city").toString();

			if (field != 0)
				nodes.append(field);
		}
		else if (xml.name() == "road")
		{
			xml.readNext();
			QStringList const & split = xml.text().toString().split(' ', QString::SkipEmptyParts);
			Node * road = newRoadNode(tile, split.length());
			for (QString const & str : split)
			{
				int side;
				int subside = 1;

				convertSide(str, side, subside);
				if (side < 0)
				{
					qWarning() << "unknown side:" << str;
					delete road;
					road = 0;
					break;
				}

				edgeConnectors[side][subside] = road;

				if (edges[side] == None)
					edges[side] = Road;
				else
				{
					qWarning() << "side not distinct";
					delete road;
					road = 0;
					break;
				}
			}

			if (road != 0)
				nodes.append(road);
		}
		else if (xml.name() == "city")
		{
			bool pennant = (xml.attributes().value("pennant") == "yes");
			xml.readNext();
			QStringList const & split = xml.text().toString().split(' ', QString::SkipEmptyParts);
			Node * city = newCityNode(tile, split.length(), pennant);
			for (QString const & str : split)
			{
				int side;
				int subside = 1;

				convertSide(str, side, subside);
				if (side < 0)
				{
					qWarning() << "unknown side:" << str;
					delete city;
					city = 0;
					break;
				}

				edgeConnectors[side][subside] = city;

				if (edges[side] == None)
					edges[side] = City;
				else
				{
					qWarning() << "side not distinct";
					delete city;
					city = 0;
					break;
				}
			}

			if (city != 0)
				nodes.append(city);
		}
		else if (xml.name() == "position")
		{
			int x = xml.attributes().value("x").toInt(&ok);
			if (ok)
			{
				int y = xml.attributes().value("y").toInt(&ok);
				if (ok)
				{
					data.hasPosition = true;
					data.position = QPoint(x, y);
				}
			}
		}
		else
		{
			qWarning() << "unsupported node type:" << xml.name();
		}

		xml.skipCurrentElement();
	}

	for (int i = 0; i < 4; ++i)
	{
		Tile::Side side = (Tile::Side)i;

		tile->createEdgeList(side);
		switch (edges[i])
		{
			case None:
				qWarning() << "Not all sides specified";
				break;
			case Field:
				tile->setEdgeNode(side, 0, edgeConnectors[i][0]);
				break;
			case City:
				tile->setEdgeNode(side, 0, edgeConnectors[i][1]);
				break;
			case Road:
				tile->setEdgeNode(side, 0, edgeConnectors[i][0]);
				tile->setEdgeNode(side, 1, edgeConnectors[i][1]);
				tile->setEdgeNode(side, 2, edgeConnectors[i][2]);
				break;
			case Cloister:
				break;
		}
	}

	tile->nodeCount = nodes.size();
	tile->nodes = new Node *[tile->nodeCount];
	for (int i = 0; i < tile->nodeCount; ++i)
		tile->nodes[i] = nodes.at(i);

	tileTemplates[set].append(tile);
	tileMetaData[tile] = data;
	tileIdentifiers[set].append(id);
//	qDebug() << this << tileIdentifiers[set].size();
}

QStringList jcz::TileFactory::getTileIdentifiers(Tile::TileSet set) const
{
	return tileIdentifiers[set];
}

QString jcz::TileFactory::getTileIdentifier(Tile::TileSet set, int type) const
{
	return tileIdentifiers[set][type];
}
