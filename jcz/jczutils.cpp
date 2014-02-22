#include "jczutils.h"

#include <QFile>

#include <QDebug>
QList<Tile *> JCZUtils::TileFactory::createPack(QString file)
{
	QList<Tile *> pack;

	QFile f(file);
	f.open(QIODevice::ReadOnly);

	QXmlStreamReader xml(&f);

	xml.readNextStartElement();
	if (xml.name() != "pack")
		qWarning() << "unknown element:" << xml.name();

	int index = 0;
	while (!xml.atEnd())
	{
		xml.readNextStartElement();

		if (xml.name() == "tile")
			createTile(xml, pack, Tile::BaseGame, index++);
		if (xml.name() != "pack")
			qWarning() << "unknown element:" << xml.name();

//		qDebug() << xml.tokenType();
//		qDebug() << xml.tokenString();
//		qDebug() << xml.name();
//		qDebug() << xml.text();
//		qDebug();
	}
	if (xml.hasError())
	{
		qWarning() << xml.errorString();
	}

	f.close();
	return pack;
}

inline Node * newFieldNode()
{
	return new Node(Field);
}

inline Node * newCityNode(uchar open, bool pennant = false)
{
	return new CityNode(open, pennant ? 1 : 0);
}

inline Node * newRoadNode()
{
	return new Node(Road);
}

inline Node * newCloisterNode()
{
	return new Node(Cloister);
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

void JCZUtils::TileFactory::createTile(QXmlStreamReader & xml, QList<Tile *> & pack, Tile::TileSet set, int type)
{
	bool ok = false;
	int count = xml.attributes().value("count").toInt(&ok);
	if (!ok || count <= 0)
		return;
	QString id = xml.attributes().value("id").toString(); //TODO
	bool starter = false;

	Tile * tile = new Tile(set, type);
	TerrainType (&edges)[4] = tile->edges;
	QList<Node *> nodes;
	Node * edgeConnectors[3][4] = { {0} };
	while (xml.readNextStartElement())
	{
		if (xml.name() == "cloister")
		{
			Node * cloister = newCloisterNode();
			nodes.append(cloister);
		}
		else if (xml.name() == "farm")
		{
			Node * field = newFieldNode();
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
			Node * road = newRoadNode();
			for (QString const & str : xml.text().toString().split(' ', QString::SkipEmptyParts))
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
			QStringList const & split = xml.text().toString().split(' ', QString::SkipEmptyParts);
			Node * city = newCityNode(split.length(), pennant);
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
			//TODO
			starter = true;
		}
		else
		{
			qWarning() << "unsupported node type:" << xml.name();
		}
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

	pack.append(tile);

	for (int i = 1; i < count; ++i)
		pack.append(new Tile(*tile));
}
