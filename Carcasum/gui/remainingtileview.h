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

#ifndef REMAININGTILEVIEW_H
#define REMAININGTILEVIEW_H

#include "tileimagefactory.h"
#include "guiIncludes.h"

namespace Ui {
class RemainingTileView;
}

class RemainingTileView : public QWidget
{
	Q_OBJECT

private:
	int type;
	int count;
	bool highlight = false;
	QPixmap pxNormal;
	QPixmap pxHl;

public:
	explicit RemainingTileView(TileTypeType type, int count, TileImageFactory * imgFactory, QWidget *parent = 0);
	~RemainingTileView();

	inline int getType() const { return type; }
	inline int getCount() const { return count; }

public slots:
	void setCount(int count);
	void setHighlight(bool hl);

private:
	QGraphicsColorizeEffect * newColorEffect();

private:
	Ui::RemainingTileView *ui;
};

#endif // REMAININGTILEVIEW_H
