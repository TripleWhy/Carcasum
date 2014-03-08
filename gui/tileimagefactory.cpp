#include "tileimagefactory.h"
#include "boardgraphicsscene.h"

TileImageFactory::TileImageFactory(jcz::TileFactory * tileFactory)
	: tileFactory(tileFactory)
{
}

TileImageFactory::~TileImageFactory()
{
	images.clear();
}

const QPixmap TileImageFactory::getImage(Tile const * tile)
{
	return getImage(tile->tileSet, tile->tileType);
}

const QPixmap TileImageFactory::getImage(Tile::TileSet tileSet, int tileType)
{
	QList<QPixmap> & imgList = images[tileSet];
	while (imgList.size() - 1 < tileType)
		imgList.append(loadImage(tileSet, imgList.size()));

	return imgList[tileType];
}

QString TileImageFactory::getMeepleFillSvg(const Node * node) const
{
	switch (node->t)
	{
		case Field:
			return ":/img/meeple/laying-fill";
		default:
			return ":/img/meeple/standing-fill";
	}
}

QString TileImageFactory::getMeepleOutlineSvg(const Node * node) const
{
	switch (node->t)
	{
		case Field:
			return ":/img/meeple/laying-outline";
		default:
			return ":/img/meeple/standing-outline";
	}
}

QColor TileImageFactory::getPlayerColor(int player) const
{
	//TODO make dynamic
	switch (player)
	{
		case 0:
			return Qt::red;
		case 1:
			return Qt::blue;
		case 2:
			return Qt::yellow;
		case 3:
			return Qt::darkGreen;
		case 4:
			return Qt::black;
		case 5:
			return Qt::gray;
		default:
			return Qt::magenta;
	}
}

QMap<uchar, QPoint> TileImageFactory::getPoints(Tile const * tile)
{
	jcz::Expansion expansion = jcz::Expansions::fromTileSet(tile->tileSet);
	if (!xmlTiles.contains(expansion))
	{
		auto && tiles = jcz::XmlParser::readTileDefinitions(expansion);
		jcz::XmlParser::readPoints(":/jcz/defaults/points.xml", tiles);
		jcz::XmlParser::readPoints(":/jcz/resources/plugins/classic/tiles/points.xml", tiles);
		xmlTiles.insert(expansion, tiles);
	}

	// :-/  All this relies on the fact that both TileFactory and XMLParser read the same file in the same order.

	jcz::XmlParser::XMLTile const & xTile = xmlTiles.value(expansion).at(tile->tileType);

	QMap<uchar, QPoint> points;
	for (uchar i = 0; i < tile->getNodeCount(); ++i)
	{
		QPoint point = xTile.features[i].point;
		point *= (BoardGraphicsScene::TILE_SIZE / 1000.0);
		points.insert(i, point);
	}

	return points;
}


QPixmap TileImageFactory::loadImage(Tile::TileSet tileSet, int tileType)
{
	QString prefix;
	switch (tileSet)
	{
		case Tile::BaseGame:
			prefix = "BaseGame";
			break;
	}

	QPixmap img(QString(":/tile/%1/%2").arg(prefix).arg(tileFactory->getTileIdentifier(tileSet, tileType)));
	Q_ASSERT(!img.isNull());
	return img.scaled(BoardGraphicsScene::TILE_SIZE, BoardGraphicsScene::TILE_SIZE, Qt::IgnoreAspectRatio, Qt::SmoothTransformation);
}
