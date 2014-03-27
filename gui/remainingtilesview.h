#ifndef REMAININGTILESVIEW_H
#define REMAININGTILESVIEW_H

#include "static.h"
#include "core/tile.h"
#include "remainingtileview.h"
#include <QWidget>
#include <QGridLayout>
#include <QVarLengthArray>
#include <map>

namespace Ui {
class RemainingTilesView;
}

class RemainingTilesView : public QWidget
{
	Q_OBJECT

private:
	QVarLengthArray<RemainingTileView *, TILE_COUNT_ARRAY_LENGTH> views;
	Game const * game;

public:
	explicit RemainingTilesView(QWidget *parent = 0);
	~RemainingTilesView();

	void clear();
	void setUp(Game const * g, TileImageFactory * imgFactory);

public slots:
	void updateView();

private:
	Ui::RemainingTilesView *ui;
};

#endif // REMAININGTILESVIEW_H
