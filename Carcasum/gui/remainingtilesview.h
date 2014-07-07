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

#ifndef REMAININGTILESVIEW_H
#define REMAININGTILESVIEW_H

#include "static.h"
#include "core/tile.h"
#include "core/nexttileprovider.h"
#include "remainingtileview.h"
#include "guiIncludes.h"
#include <map>

namespace Ui {
class RemainingTilesView;
}

class RemainingTilesView : public QWidget, public NextTileProvider
{
	Q_OBJECT

private:
	QVarLengthArray<RemainingTileView *, TILE_COUNT_ARRAY_LENGTH> views;
	QList<RemainingTileView *> discardedViews;
	Game const * game;
	TileImageFactory * imgFactory;

	bool running = false;
	bool ready = false;
	int type = -1;

public:
	explicit RemainingTilesView(QWidget *parent = 0);
	~RemainingTilesView();

	void clear();
	void setUp(Game const * g, TileImageFactory * imgFactory);
	virtual int nextTile(Game const * game);

protected:
	virtual void leaveEvent(QEvent *);
	virtual void mouseMoveEvent(QMouseEvent * event);
	virtual void mousePressEvent(QMouseEvent * event);

public slots:
	void updateView();

private:
	RemainingTileView * getViewAt(QPoint const & pos);

private:
	Ui::RemainingTilesView *ui;
};

#endif // REMAININGTILESVIEW_H
