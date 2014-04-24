#include "tileimagefactory.h"
#include "boardgraphicsscene.h"

#include <QtSvg/QSvgRenderer>
#include <QPainter>

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
	return getImage(tile->tileType);
}

const QPixmap TileImageFactory::getImage(TileTypeType tileType)
{
	Tile::TileSet set = Util::getSet(tileType);
	TileTypeType localType = Util::toLocalType(tileType);
	return getImage(set, localType);
}

const QPixmap TileImageFactory::getImage(Tile::TileSet set, TileTypeType localType)
{
	QList<QPixmap> & imgList = images[set];
	while (imgList.size() - 1 < localType)
		imgList.append(loadImage(set, (TileTypeType)imgList.size()));

	return imgList[localType];
}

QString TileImageFactory::getMeepleFillSvg(const Node * node) const
{
	switch (node->getTerrain())
	{
		case Field:
			return ":/img/meeple/laying-fill";
		default:
			return ":/img/meeple/standing-fill";
	}
}

QString TileImageFactory::getMeepleOutlineSvg(const Node * node) const
{
	switch (node->getTerrain())
	{
		case Field:
			return ":/img/meeple/laying-outline";
		default:
			return ":/img/meeple/standing-outline";
	}
}

QString TileImageFactory::getMeepleFillSvgStanding() const
{
	return ":/img/meeple/standing-fill";
}

QString TileImageFactory::getMeepleOutlineSvgStanding() const
{
	return ":/img/meeple/standing-outline";
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

QPixmap TileImageFactory::generateMeepleStanding(int size, QColor color)
{
	QSvgRenderer renderer(getMeepleFillSvgStanding());
	qreal r = renderer.viewBoxF().width() / renderer.viewBoxF().height();
	int w = size;
	int h = size;
	if (r < 1.0)
		w = (int)round(r * h);
	else
		h = (int)round(w / r);
	
	QPixmap pixmap(w, h);
	pixmap.fill(Qt::transparent);
	
	QPainter painter(&pixmap);
	renderer.render(&painter);
	
	painter.setCompositionMode(QPainter::CompositionMode_SourceIn);
	painter.fillRect(pixmap.rect(), color);
	
	painter.setCompositionMode(QPainter::CompositionMode_SourceOver);
	renderer.load(getMeepleOutlineSvgStanding());
	renderer.render(&painter);
	
	return pixmap;
}

QMap<uchar, QPoint> TileImageFactory::getPoints(Tile const * tile)
{
	jcz::Expansion expansion = jcz::Expansions::fromTileSet(Util::getSet(tile->tileType));
	if (!xmlTiles.contains(expansion))
	{
		auto && tiles = jcz::XmlParser::readTileDefinitions(expansion);
		jcz::XmlParser::readPoints(":/jcz/defaults/points.xml", tiles);
		jcz::XmlParser::readPoints(":/jcz/resources/plugins/classic/tiles/points.xml", tiles);
		xmlTiles.insert(expansion, tiles);
	}

	// :-/  All this relies on the fact that both TileFactory and XMLParser read the same file in the same order.

	jcz::XmlParser::XMLTile const & xTile = xmlTiles.value(expansion).at(Util::toLocalType(tile->tileType));

	QMap<uchar, QPoint> points;
	for (uchar i = 0; i < tile->getNodeCount(); ++i)
	{
		QPoint point = xTile.features[i].point;
		point *= (BOARD_TILE_SIZE / 1000.0);
		points.insert(i, point);
	}

	return points;
}


QPixmap TileImageFactory::loadImage(Tile::TileSet tileSet, TileTypeType localType)
{
	QString prefix;
	switch (tileSet)
	{
		case Tile::BaseGame:
			prefix = "BaseGame";
			break;
	}

	QPixmap img(QString(":/tile/%1/%2").arg(prefix).arg(tileFactory->getTileIdentifier(tileSet, localType)));
	Q_ASSERT(!img.isNull());
	return img.scaled(BOARD_TILE_SIZE, BOARD_TILE_SIZE, Qt::IgnoreAspectRatio, Qt::SmoothTransformation);
}
