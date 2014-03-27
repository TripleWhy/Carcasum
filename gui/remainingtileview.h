#ifndef REMAININGTILEVIEW_H
#define REMAININGTILEVIEW_H

#include "tileimagefactory.h"

#include <QWidget>
#include <QGraphicsColorizeEffect>

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
	QGraphicsColorizeEffect effect;

public:
	explicit RemainingTileView(TileTypeType type, int count, TileImageFactory * imgFactory, QWidget *parent = 0);
	~RemainingTileView();

	inline int getType() const { return type; }
	inline int getCount() const { return count; }

public slots:
	void setCount(int count);
	void setHighlight(bool hl);

private:
	Ui::RemainingTileView *ui;
};

#endif // REMAININGTILEVIEW_H
