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
	effect.setColor(Qt::black);

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
		ui->tileLabel->setGraphicsEffect(&effect);
	else
		ui->tileLabel->setGraphicsEffect(0);
	ui->countLabel->setText(QString("%1x").arg(count));
	this->count = count;
}

void RemainingTileView::setHighlight(bool hl)
{
	ui->tileLabel->setPixmap(hl ? pxHl : pxNormal);
}
