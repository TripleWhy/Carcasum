/*
	This file is part of Carcasum.

	Carcasum is free software: you can redistribute it and/or modify
	it under the terms of the GNU Affero General Public License as published by
	the Free Software Foundation, either version 3 of the License, or
	(at your option) any later version.

	Carcasum is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU Affero General Public License for more details.

	You should have received a copy of the GNU Affero General Public License
	along with Carcasum.  If not, see <http://www.gnu.org/licenses/>.
*/

#ifndef PLAYERINFOVIEW_H
#define PLAYERINFOVIEW_H

#include "static.h"
#include "tileimagefactory.h"
#include "guiIncludes.h"

namespace Ui {
class PlayerInfoView;
}

class PlayerInfoView : public QWidget
{
Q_OBJECT

private:
	int playerIndex = -1;
	Game const * game = 0;
	TileImageFactory * imgFactory = 0;
	
	QLabel * meepleLabels[MEEPLE_COUNT];
	QPalette normPalette;
	QPalette hlPalette;
	
public:
	explicit PlayerInfoView(QWidget *parent = 0);
	explicit PlayerInfoView(int player, Game const * game, TileImageFactory * tif, QWidget *parent = 0);
	~PlayerInfoView();
	void setPlayer(int player, const Game * game, TileImageFactory * tif);
	void setPlayerName(QString const & name);

public slots:
	void updateView();
	void displayTile(int player, int tileType);

private:
	Ui::PlayerInfoView *ui;
};

#if PLAYERINFOVIEW_SCALEABLE
class PIVLabel : public QLabel
{
public:
public:
	explicit PIVLabel(QWidget *parent=0, Qt::WindowFlags f=0) : QLabel(parent, f) {}
	explicit PIVLabel(const QString &text, QWidget *parent=0, Qt::WindowFlags f=0) : QLabel(text, parent, f) {}
protected:
	virtual void paintEvent(QPaintEvent * e)
	{
		if (!hasScaledContents())
			return QLabel::paintEvent(e);

		QPixmap const * px = pixmap();
		if (px == 0 || px->isNull())
			return QLabel::paintEvent(e);

		QPainter painter(this);
		painter.setRenderHints(QPainter::Antialiasing | QPainter::TextAntialiasing | QPainter::SmoothPixmapTransform | QPainter::HighQualityAntialiasing);
		drawFrame(&painter);
		QRect cr = contentsRect();
		painter.drawPixmap(cr, *px);
	}
};
#else
typedef QLabel PIVLabel;
#endif

#endif // PLAYERINFOVIEW_H
