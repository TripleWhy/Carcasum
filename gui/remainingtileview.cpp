#include "remainingtileview.h"
#include "ui_remainingtileview.h"

RemainingTileView::RemainingTileView(TileTypeType type, int count, TileImageFactory * imgFactory, QWidget *parent) :
    QWidget(parent),
    ui(new Ui::RemainingTileView)
{
	ui->setupUi(this);

	QPixmap const & p = imgFactory->getImage(type);
	ui->tileLabel->setPixmap(p.scaled(RTILE_TILE_SIZE, RTILE_TILE_SIZE, Qt::KeepAspectRatio, Qt::SmoothTransformation));
	ui->nameLabel->setText(imgFactory->getName(type));
	setCount(count);
}

RemainingTileView::~RemainingTileView()
{
	delete ui;
}


// From http://qt-project.org/faq/answer/how_can_i_convert_a_colored_qpixmap_into_a_grayscaled_qpixmap
inline QPixmap gray(QPixmap const * pixmap)
{
	QImage image = pixmap->toImage();
	QRgb col;
	int gray;
	int width = pixmap->width();
	int height = pixmap->height();
	for (int i = 0; i < width; ++i)
	{
		for (int j = 0; j < height; ++j)
		{
			col = image.pixel(i, j);
			gray = qGray(col);
			image.setPixel(i, j, qRgb(gray, gray, gray));
		}
	}
	return QPixmap::fromImage(image);
}

void RemainingTileView::setCount(int count)
{
	if (count == 0)
		ui->tileLabel->setPixmap( gray(ui->tileLabel->pixmap()) );
	ui->countLabel->setText(QString("%1x").arg(count));
}
