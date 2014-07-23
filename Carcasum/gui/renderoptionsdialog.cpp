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
