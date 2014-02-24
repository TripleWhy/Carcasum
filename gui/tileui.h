#ifndef TILEUI_H
#define TILEUI_H

#include "core/tile.h"
#include "core/board.h"
#include "jcz/jczutils.h"

#include <QLabel>
#include <QMouseEvent>

class TileUI : public QLabel
{
Q_OBJECT
public:
	static int const TILE_SIZE = 50;

private:
	QPixmap baseImg;
	QPixmap img;
	bool open = false;
	uint x, y;
	const Tile * openTile = 0;
	Tile::Side openOrientation;
	JCZUtils::TileFactory * tileFactory;

public:
	TileUI(Tile const * tile, JCZUtils::TileFactory * tileFactory, QWidget * parent = 0);
	TileUI(uint x, uint y, JCZUtils::TileFactory * tileFactory, QWidget * parent = 0);

	void setOpenTile(const Tile * const tile);
	Board::TilePlacement getTilePlacement();

	virtual QSize sizeHint() const;

protected:
	virtual void mousePressEvent(QMouseEvent * ev);
	virtual void enterEvent(QEvent * event);
	virtual void leaveEvent(QEvent * event);

private:
	void loadImage(Tile::TileSet tileSet, int tileType);
	void setRotatedImage(Tile::Side orientation);

signals:
	void tilePlaced();
};

#endif // TILEUI_H
