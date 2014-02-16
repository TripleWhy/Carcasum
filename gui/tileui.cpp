#include "tileui.h"

TileUI::TileUI(Tile const * tile, QWidget * parent)
	: QLabel(parent)
{
	QString prefix;
	switch (tile->tileSet)
	{
		case Tile::BaseGame:
			prefix = "BaseGame";
			break;
	}
	img.load(QString(":/tile/%1/%2").arg(prefix).arg(char('A' + tile->tileType)));

	if (tile->orientation != 0)
	{
		qDebug() << "type:" << tile->tileType << "orientation:" << tile->orientation;
		img = img.transformed(QTransform().rotate(tile->orientation * 90));
	}

	setPixmap(img);
	setScaledContents(true);
}

QSize TileUI::sizeHint() const
{
	return img.size();
}

void TileUI::mousePressEvent(QMouseEvent * ev)
{
	if (ev->buttons() != Qt::LeftButton)
		return;

	ev->accept();
	qDebug() << ev->pos();
}
