#ifndef JCZUTILS_H
#define JCZUTILS_H

#include "core/tile.h"

#include <QXmlStreamReader>

class Tile;

namespace JCZUtils
{

class TileFactory
{
public:
	static QList<Tile *> createPack(QString file);
private:
	static void createTile(QXmlStreamReader & xml, QList<Tile *> & pack, Tile::TileSet set, int type);
};

}

#endif // JCZUTILS_H
