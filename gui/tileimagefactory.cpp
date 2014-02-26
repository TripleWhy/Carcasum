#include "tileimagefactory.h"
#include "boardgraphicsscene.h"

TileImageFactory::TileImageFactory(JCZUtils::TileFactory * tileFactory)
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
