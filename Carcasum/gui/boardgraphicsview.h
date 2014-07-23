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
