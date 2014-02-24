#ifndef JCZUTILS_H
#define JCZUTILS_H

#include "core/tile.h"

#include <QXmlStreamReader>
#include <QHash>
#include <QPoint>

class Tile;

namespace JCZUtils
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
	QList<Tile *> createPack(Tile::TileSets tileSets);
	void createPack(Tile::TileSet tileSet, QList<Tile *> & pack);
private:
	void readXMLPack(Tile::TileSet set);
	void readXMLPack(QString file, Tile::TileSet tileSet);
	void readXMLTile(QXmlStreamReader & xml, Tile::TileSet set);

public:
	QStringList getTileIdentifiers(Tile::TileSet set) const;
	QString getTileIdentifier(Tile::TileSet set, int type) const;
};

}

#endif // JCZUTILS_H
