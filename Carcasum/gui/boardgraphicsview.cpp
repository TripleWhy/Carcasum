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
	setMouseTracking(true);
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
// extended by zoom limitation
void BoardGraphicsView::wheelEvent(QWheelEvent* event)
{
	event->accept();

	qreal currentScale = transform().m11();
	int delta = event->delta();
	if (currentScale >= BOARDGRAPHICSVIEW_MAX_ZOOM && delta >= 0)
		return;
	if (currentScale <= BOARDGRAPHICSVIEW_MIN_ZOOM && delta <= 0)
		return;

	const QPointF p0scene = mapToScene(event->pos());

	qreal factor = std::pow(BOARDGRAPHICSVIEW_ZOOM_SPEED, delta);
	scale(factor, factor);

	const QPointF p1mouse = mapFromScene(p0scene);
	const QPointF move = p1mouse - event->pos(); // The move
	horizontalScrollBar()->setValue((int)round(move.x() + horizontalScrollBar()->value()));
	verticalScrollBar()->setValue((int)round(move.y() + verticalScrollBar()->value()));
}
