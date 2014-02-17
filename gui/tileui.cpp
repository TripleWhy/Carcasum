#include "tileui.h"

TileUI::TileUI(Tile const * tile, QWidget * parent)
	: QLabel(parent)
{
	if (tile == 0)
	{
		setStyleSheet("QLabel { background: #30FFFFFF; }");
	}
	else
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
	//		qDebug() << "TileUI::TileUI()   type:" << tile->tileType << "orientation:" << tile->orientation;
			img = img.transformed(QTransform().rotate(tile->orientation * 90));
		}
#if TILE_SIZE > 0
		img = img.scaled(TILE_SIZE, TILE_SIZE, Qt::IgnoreAspectRatio, Qt::SmoothTransformation);
#endif

		setPixmap(img);
		setScaledContents(true);
	}
}

QSize TileUI::sizeHint() const
{
	if (img.isNull())
		return QLabel::sizeHint();
	return img.size();
}

void TileUI::mousePressEvent(QMouseEvent * ev)
{
	if (ev->buttons() != Qt::LeftButton)
		return;

	ev->accept();
	qDebug() << ev->pos();
}
