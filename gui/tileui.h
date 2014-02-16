#ifndef TILEUI_H
#define TILEUI_H

#include "../core/tile.h"

#include <QLabel>
#include <QMouseEvent>

class TileUI : public QLabel
{
private:
	QPixmap img;

public:
	TileUI(Tile const * tile, QWidget * parent = 0);

	virtual QSize sizeHint() const;

protected:
	virtual void mousePressEvent(QMouseEvent * ev);
};

#endif // TILEUI_H
