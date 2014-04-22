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
	Game const * game;

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
