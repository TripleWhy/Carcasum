#ifndef REMAININGTILEVIEW_H
#define REMAININGTILEVIEW_H

#include "tileimagefactory.h"

#include <QWidget>

namespace Ui {
class RemainingTileView;
}

class RemainingTileView : public QWidget
{
	Q_OBJECT

public:
	explicit RemainingTileView(TileTypeType type, int count, TileImageFactory * imgFactory, QWidget *parent = 0);
	~RemainingTileView();

public slots:
	void setCount(int count);

private:
	Ui::RemainingTileView *ui;
};

#endif // REMAININGTILEVIEW_H
