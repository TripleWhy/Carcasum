/*
	This file is part of Carcasum.

	Carcasum is free software: you can redistribute it and/or modify
	it under the terms of the GNU Affero General Public License as published by
	the Free Software Foundation, either version 3 of the License, or
	(at your option) any later version.

	Carcasum is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU Affero General Public License for more details.

	You should have received a copy of the GNU Affero General Public License
	along with Carcasum.  If not, see <http://www.gnu.org/licenses/>.
*/

#ifndef TILEFACTORY_H
#define TILEFACTORY_H

#include "core/tile.h"

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wconversion"
#include <QXmlStreamReader>
#include <QHash>
#include <QPoint>
#include <QReadWriteLock>
#pragma GCC diagnostic pop

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

	QReadWriteLock lock;
	QHash<Tile::TileSet, QStringList> tileIdentifiers;
	QHash<Tile::TileSet, QList<Tile *>> tileTemplates;
	QHash<Tile *, TileMetaData> tileMetaData;
	bool printUnsupportedNodeTypes;

public:
	TileFactory(bool printUnsupportedNodeTypes = true);
	~TileFactory();
	QList<Tile *> createPack(Tile::TileSets tileSets, const Game * g);
	void createPack(Tile::TileSet tileSet, QList<Tile *> & pack, const Game * g);

private:
	void readXMLPack(Tile::TileSet set, const Game * g);
	void readXMLPack(QString file, Tile::TileSet tileSet, const Game * g);
	void readXMLTile(QXmlStreamReader & xml, Tile::TileSet set, const Game * g);

public:
	QStringList getTileIdentifiers(Tile::TileSet set) const;
	QString getTileIdentifier(Tile::TileSet set, TileTypeType localType) const;
};

}

#endif // TILEFACTORY_H
