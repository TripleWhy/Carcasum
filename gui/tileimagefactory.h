#ifndef TILEIMAGEFACTORY_H
#define TILEIMAGEFACTORY_H

#include "core/tile.h"
#include "jcz/tilefactory.h"
#include "jcz/xmlparser.h"

#include <QHash>
#include <QPixmap>

class TileImageFactory
{
private:
	jcz::TileFactory * tileFactory;
	QHash<Tile::TileSet, QList<QPixmap>> images;

	QHash<jcz::Expansion, QList<jcz::XmlParser::XMLTile>> xmlTiles;

public:
	TileImageFactory(jcz::TileFactory * tileFactory);
	~TileImageFactory();
	const QPixmap getImage(const Tile * tile);
	const QPixmap getImage(Tile::TileSet tileSet, int tileType);
	QString getMeepleFillSvg(Node const * node) const;
	QString getMeepleOutlineSvg(Node const * node) const;
	QColor getPlayerColor(int player) const;

	QMap<uchar, QPoint> getPoints(Tile const * tile);

private:
	QPixmap loadImage(Tile::TileSet tileSet, int tileType);
};

#endif // TILEIMAGEFACTORY_H
