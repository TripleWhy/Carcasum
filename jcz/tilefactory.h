#ifndef TILEFACTORY_H
#define TILEFACTORY_H

#include "core/tile.h"

#include <QXmlStreamReader>
#include <QHash>
#include <QPoint>

class Tile;

namespace jcz
{

class TileFactory
{
private:
	struct TileMetaData
	{
		uint count = 0;
		bool hasPosition = false;
		QPoint position;
	};

	QHash<Tile::TileSet, QStringList> tileIdentifiers;
	QHash<Tile::TileSet, QList<Tile *>> tileTemplates;
	QHash<Tile *, TileMetaData> tileMetaData;

public:
	~TileFactory();
	QList<Tile *> createPack(Tile::TileSets tileSets, const Game * g);
	void createPack(Tile::TileSet tileSet, QList<Tile *> & pack, const Game * g);

private:
	void readXMLPack(Tile::TileSet set, const Game * g);
	void readXMLPack(QString file, Tile::TileSet tileSet, const Game * g);
	void readXMLTile(QXmlStreamReader & xml, Tile::TileSet set, const Game * g);

public:
	QStringList getTileIdentifiers(Tile::TileSet set) const;
	QString getTileIdentifier(Tile::TileSet set, int type) const;
};

}

#endif // TILEFACTORY_H
