#ifndef TILEIMAGEFACTORY_H
#define TILEIMAGEFACTORY_H

#include "core/tile.h"
#include "jcz/tilefactory.h"
#include "jcz/xmlparser.h"
#include <array>
#include <QHash>
#include <QPixmap>
#include <QBuffer>
#include <quazip/quazip.h>
#include <quazip/quazipfile.h>

class TileImageFactory
{
private:
	jcz::TileFactory * tileFactory;
	QHash<Tile::TileSet, QList<QPixmap>> images;

	QHash<jcz::Expansion, QList<jcz::XmlParser::XMLTile>> xmlTiles;
	QByteArray zipData;
	std::array<QColor, MAX_PLAYERS> playerColors = {{Qt::red, Qt::blue, Qt::yellow, Qt::darkGreen, Qt::black, Qt::gray}};

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
	void setPlayerColor(int player, QColor const & color);
	QColor getPlayerColor(int player) const;
	QPixmap generateMeepleStanding(int size, QColor color);

	QMap<uchar, QPoint> getPoints(Tile const * tile);
	QString getName(TileTypeType type) { Q_UNUSED(type); return QString(); }	//TODO?
	static QString zipFileName();

private:
	QPixmap loadImage(Tile::TileSet tileSet, TileTypeType localType);
	QByteArray getPluginData();
	QPixmap loadPluginImage(QString const & path);

private:
	class PluginFileMgr
	{
	private:
		QBuffer * buffer = 0;
		QuaZip * qz = 0;
		QuaZipFile * file = 0;
		bool ok = false;

	public:
		PluginFileMgr(QByteArray & zipData, QString const & path, bool open = false)
		{
			if (zipData.isNull())
				return;

			buffer = new QBuffer(&zipData);
			qz = new QuaZip(buffer);
			qz->open(QuaZip::mdUnzip);
			file = new QuaZipFile(qz);
			ok = qz->setCurrentFile(path);
			if (ok && open)
				file->open(QIODevice::ReadOnly);
		}
		~PluginFileMgr()
		{
			if (ok && file != 0 && file->isOpen())
				file->close();
			if (qz != 0)
				qz->close();

			delete file;
			delete qz;
			delete buffer;
		}
		bool fileExists()
		{
			return ok;
		}
		QuaZipFile * getFile()
		{
			return file;
		}
	};
};

#endif // TILEIMAGEFACTORY_H