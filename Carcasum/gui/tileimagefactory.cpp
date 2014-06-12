#include "tileimagefactory.h"
#include "boardgraphicsscene.h"
#include "core/util.h"
#include <QtSvg/QSvgRenderer>
#include <QPainter>
#include <QDir>

TileImageFactory::TileImageFactory(jcz::TileFactory * tileFactory)
	: tileFactory(tileFactory)
{
	zipData = getPluginData();
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

void TileImageFactory::setPlayerColor(int player, const QColor & color)
{
	if (player < 0 || player >= (int)playerColors.size())
		return;
	else
		playerColors[player] = color;
}

QColor TileImageFactory::getPlayerColor(int player) const
{
	if (player < 0 || player >= (int)playerColors.size())
		return Qt::magenta;
	else
		return playerColors[player];
}

void TileImageFactory::setPlayerName(int player, const QString & name)
{
	if (player < 0 || player >= (int)playerNames.size())
		return;
	else
		playerNames[player] = name;
}

QString TileImageFactory::getPlayerName(int player) const
{
	if (player < 0 || player >= (int)playerNames.size())
		return QString("Player %1").arg(player);
	else
		return playerNames[player];
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
		QFile file(":/jcz/defaults/points.xml");
		jcz::XmlParser::readPoints(&file, tiles);
		QFile file2(":/jcz/resources/plugins/classic/tiles/points.xml");
		jcz::XmlParser::readPoints(&file2, tiles);

#ifndef CLASSIC_TILES
		PluginFileMgr mgr(zipData, "tiles/points.xml");
		if (mgr.fileExists())
			jcz::XmlParser::readPoints(mgr.getFile(), tiles);
#endif
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

QString TileImageFactory::zipFileName()
{
	return "JCloisterZone-2.6.zip";
}


QPixmap TileImageFactory::loadImage(Tile::TileSet tileSet, TileTypeType localType)
{
	QString prefix;
	switch (tileSet)
	{
		case Tile::BaseGame:
			prefix = "BA";
			break;
	}

	QString path = QString("tiles/%1/%2").arg(prefix).arg(tileFactory->getTileIdentifier(tileSet, localType));

	QPixmap img = loadPluginImage(QString("%1.jpg").arg(path));
	if (img.isNull())
		img.load(QString(":/%1").arg(path));
	Q_ASSERT(!img.isNull());
	return img.scaled(BOARD_TILE_SIZE, BOARD_TILE_SIZE, Qt::IgnoreAspectRatio, Qt::SmoothTransformation);
}

QByteArray TileImageFactory::getPluginData()
{
#ifdef CLASSIC_TILES
	return QByteArray();
#else
	QDir dir = QDir(QCoreApplication::applicationDirPath());
	QFileInfo fi(dir, QString(zipFileName()));
	QByteArray data;
	QByteArray pluginData;
	if (fi.exists())
	{
		QFile file(fi.absoluteFilePath());
		file.open(QIODevice::ReadOnly);
		data = file.readAll();
		file.close();
	}
	else
	{
		return pluginData;
	}

	QBuffer buffer(&data);
	QuaZip qz(&buffer);
	if (!qz.open(QuaZip::mdUnzip))
	{
		qDebug() << "qz.open error:" << qz.getZipError();
		return pluginData;
	}
	bool found = false;
	while (qz.goToNextFile())
	{
		QString name = qz.getCurrentFileName();
		if (name.endsWith("plugins/classic.jar"))
		{
			found = true;
			break;
		}
	}
	if (found)
	{
		QuaZipFile zip(&qz);
		zip.open(QIODevice::ReadOnly);
		pluginData = zip.readAll();
		zip.close();
	}
	qz.close();

	return pluginData;
#endif
}

QPixmap TileImageFactory::loadPluginImage(QString const & path)
{
	QPixmap px;

#ifdef CLASSIC_TILES
	Q_UNUSED(path);
#else
	PluginFileMgr mgr(zipData, path, true);
	if (!mgr.fileExists())
		return px;

	QuaZipFile * file = mgr.getFile();
	px.loadFromData(file->readAll());
#endif
	return px;
}
