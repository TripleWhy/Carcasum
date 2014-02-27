#ifndef TILEIMAGEFACTORY_H
#define TILEIMAGEFACTORY_H

#include "core/tile.h"
#include "jcz/tilefactory.h"

#include <QHash>
#include <QPixmap>

class TileImageFactory
{
private:
	jcz::TileFactory * tileFactory;
	QHash<Tile::TileSet, QList<QPixmap>> images;

public:
	TileImageFactory(jcz::TileFactory * tileFactory);
	~TileImageFactory();
	const QPixmap getImage(const Tile * tile);
	const QPixmap getImage(Tile::TileSet tileSet, int tileType);

private:
	QPixmap loadImage(Tile::TileSet tileSet, int tileType);
};

#endif // TILEIMAGEFACTORY_H
