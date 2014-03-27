#include "boardgraphicsview.h"

#include "boardgraphicsscene.h"

#include <QMouseEvent>
#include <QScrollBar>

BoardGraphicsView::BoardGraphicsView(QWidget *parent) :
    QGraphicsView(parent)
{
	setRenderHints(QPainter::Antialiasing | QPainter::TextAntialiasing | QPainter::SmoothPixmapTransform | QPainter::HighQualityAntialiasing);

	qreal const s = BOARDVIEW_INITIAL_TILE_SIZE / qreal(BOARD_TILE_SIZE);
	scale(s, s);
}

void BoardGraphicsView::mousePressEvent(QMouseEvent *event)
{
	if (event->buttons().testFlag(Qt::MidButton))
		lastPos = event->pos();
	QGraphicsView::mousePressEvent(event);
}

void BoardGraphicsView::mouseMoveEvent(QMouseEvent *event)
{
	if (event->buttons().testFlag(Qt::MidButton))
	{
		QScrollBar *hBar = horizontalScrollBar();
		QScrollBar *vBar = verticalScrollBar();
		QPoint delta = event->pos() - lastPos;
		lastPos = event->pos();
		hBar->setValue(hBar->value() + (isRightToLeft() ? delta.x() : -delta.x()));
		vBar->setValue(vBar->value() - delta.y());
	}
	QGraphicsView::mouseMoveEvent(event);
}

// from http://stackoverflow.com/questions/19113532/qgraphicsview-zooming-in-and-out-under-mouse-position-using-mouse-wheel#20802191
void BoardGraphicsView::wheelEvent(QWheelEvent* event)
{
	const QPointF p0scene = mapToScene(event->pos());

	qreal factor = std::pow(1.002, event->delta());
	scale(factor, factor);

	const QPointF p1mouse = mapFromScene(p0scene);
	const QPointF move = p1mouse - event->pos(); // The move
	horizontalScrollBar()->setValue(move.x() + horizontalScrollBar()->value());
	verticalScrollBar()->setValue(move.y() + verticalScrollBar()->value());
}
