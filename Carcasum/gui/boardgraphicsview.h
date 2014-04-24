#ifndef BOARDGRAPHICSVIEW_H
#define BOARDGRAPHICSVIEW_H

#include "guiIncludes.h"

class BoardGraphicsView : public QGraphicsView
{
private:
	QPoint lastPos;

public:
	explicit BoardGraphicsView(QWidget * parent = 0);

protected:
	virtual void mousePressEvent(QMouseEvent * event);
	virtual void mouseMoveEvent(QMouseEvent * event);
	virtual void wheelEvent(QWheelEvent * event);
};

#endif // BOARDGRAPHICSVIEW_H
