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

#include "remainingtileview.h"
#include "ui_remainingtileview.h"
#include <QPainter>
#include <QGraphicsColorizeEffect>

RemainingTileView::RemainingTileView(TileTypeType type, int count, TileImageFactory * imgFactory, QWidget *parent) :
    QWidget(parent),
    type(type),
    count(count),
    ui(new Ui::RemainingTileView)
{
	ui->setupUi(this);

	QPixmap const & p = imgFactory->getImage(type);
	pxNormal = p.scaled(RTILE_TILE_SIZE, RTILE_TILE_SIZE, Qt::KeepAspectRatio, Qt::SmoothTransformation);
	ui->tileLabel->setPixmap(pxNormal);
	ui->nameLabel->setText(imgFactory->getName(type));
	setCount(count);

	pxHl = pxNormal;
	QPainter painter(&pxHl);
	painter.setCompositionMode(QPainter::CompositionMode_Overlay);
	painter.setOpacity(0.3);
	painter.fillRect(pxHl.rect(), Qt::white);
}

RemainingTileView::~RemainingTileView()
{
	delete ui;
}

void RemainingTileView::setCount(int count)
{
	if (count == 0)
	{
		if (ui->tileLabel->graphicsEffect() == 0)
			ui->tileLabel->setGraphicsEffect(newColorEffect());	// Widget takes ownership, so we need a new effect for every widget.
	}
	else
		ui->tileLabel->setGraphicsEffect(0);

	if (count < 0)
		ui->countLabel->setText(QString(""));
	else
		ui->countLabel->setText(QString("%1x").arg(count));
	this->count = count;
}

void RemainingTileView::setHighlight(bool hl)
{
	ui->tileLabel->setPixmap(hl ? pxHl : pxNormal);
}

QGraphicsColorizeEffect *RemainingTileView::newColorEffect()
{
	QGraphicsColorizeEffect * effect = new QGraphicsColorizeEffect();
	effect->setColor(Qt::black);
	return effect;
}
