#include "renderoptionsdialog.h"
#include "ui_renderoptionsdialog.h"

RenderOptionsDialog::RenderOptionsDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::RenderOptionsDialog)
{
	ui->setupUi(this);
}

RenderOptionsDialog::~RenderOptionsDialog()
{
	delete ui;
}

int RenderOptionsDialog::exec(int maxSteps)
{
	ui->removeTilesBox->setMaximum(maxSteps);
	return QDialog::exec();
}

int RenderOptionsDialog::getRemoveLast()
{
	return ui->removeTilesBox->value();
}

bool RenderOptionsDialog::getRenderOpenTiles()
{
	return ui->openTilesBox->isChecked();
}

bool RenderOptionsDialog::getRenderFrames()
{
	return ui->framesBox->isChecked();
}

bool RenderOptionsDialog::getRenderPlayers()
{
	return ui->playersBox->isChecked();
}

bool RenderOptionsDialog::getRenderNextTile()
{
	return ui->nextTileBox->isChecked();
}
