#include "tileui.h"

#include <QGraphicsOpacityEffect>

TileUI::TileUI(Tile const * tile, QWidget * parent)
	: QLabel(parent)
{
	if (tile == 0)
	{
		setStyleSheet("QLabel { background: #50FFFFFF; }");
		open = true;
	}
	else
	{
		open = false;

		loadImage(tile->tileSet, tile->tileType);
		setRotatedImage(tile->orientation);
		setPixmap(img);
	}
	setScaledContents(true);
}

TileUI::TileUI(uint x, uint y, QWidget * parent)
	: TileUI(0, parent)
{
	this->x = x;
	this->y = y;
	auto opacity = new QGraphicsOpacityEffect(this);
	setGraphicsEffect(opacity);
	opacity->setOpacity(0.7);
}

void TileUI::setOpenTile(const Tile * const tile)
{
	openTile = tile;
	loadImage(openTile->tileSet, openTile->tileType);
	openOrientation = (Tile::Side)0;
	setRotatedImage(openOrientation);
}

Board::TilePlacement TileUI::getTilePlacement()
{
	return Board::TilePlacement{x, y, openOrientation};
}

QSize TileUI::sizeHint() const
{
	if (img.isNull())
		return QLabel::sizeHint();
	return img.size();
}

void TileUI::mousePressEvent(QMouseEvent * ev)
{
	if (!open || openTile == 0)
		return;

	ev->accept();

	if (ev->button() == Qt::LeftButton)
	{
		emit tilePlaced();
	}
	else if (ev->button() == Qt::RightButton)
	{
		openOrientation = (Tile::Side)((openOrientation + 1) % 4);
		setRotatedImage(openOrientation);
		if (pixmap() != 0)
			setPixmap(img);
	}
}

void TileUI::enterEvent(QEvent * event)
{
	if (!open || openTile == 0)
		return;
	setPixmap(img);
}

void TileUI::leaveEvent(QEvent * event)
{
	if (!open || openTile == 0)
		return;
	setPixmap(QPixmap());
}

void TileUI::loadImage(Tile::TileSet tileSet, int tileType)
{
	QString prefix;
	switch (tileSet)
	{
		case Tile::BaseGame:
			prefix = "BaseGame";
			break;
	}
	baseImg.load(QString(":/tile/%1/%2").arg(prefix).arg(char('A' + tileType)));
}

void TileUI::setRotatedImage(Tile::Side orientation)
{
	img = baseImg.transformed(QTransform().rotate(orientation * 90));
#if TILE_SIZE > 0
	img = img.scaled(TILE_SIZE, TILE_SIZE, Qt::IgnoreAspectRatio, Qt::SmoothTransformation);
#endif
}
