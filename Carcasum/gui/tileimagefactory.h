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
	QByteArray zipData;

public:
	TileImageFactory(jcz::TileFactory * tileFactory);
	~TileImageFactory();
	const QPixmap getImage(const Tile * tile);
	const QPixmap getImage(TileTypeType tileType);
	const QPixmap getImage(Tile::TileSet set, TileTypeType localType);
	QString getMeepleFillSvg(Node const * node) const;
	QString getMeepleOutlineSvg(Node const * node) const;
	QString getMeepleFillSvgStanding() const;
	QString getMeepleOutlineSvgStanding() const;
	QColor getPlayerColor(int player) const;
	QPixmap generateMeepleStanding(int size, QColor color);

	QMap<uchar, QPoint> getPoints(Tile const * tile);
	QString getName(TileTypeType type) { Q_UNUSED(type); return QString(); }	//TODO?

private:
	QPixmap loadImage(Tile::TileSet tileSet, TileTypeType localType);
	QByteArray getPluginData();
	QPixmap loadPluginImage(QString const & path);
};

#endif // TILEIMAGEFACTORY_H
