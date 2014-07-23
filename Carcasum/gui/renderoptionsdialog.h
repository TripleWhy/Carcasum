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

#ifndef RENDEROPTIONSDIALOG_H
#define RENDEROPTIONSDIALOG_H

#include "guiIncludes.h"

namespace Ui {
class RenderOptionsDialog;
}

class RenderOptionsDialog : public QDialog
{
	Q_OBJECT

public:
	explicit RenderOptionsDialog(QWidget *parent = 0);
	~RenderOptionsDialog();

	virtual int exec(int maxSteps = 72);
	int getRemoveLast();
	bool getRenderOpenTiles();
	bool getRenderFrames();
	bool getRenderPlayers();
	bool getRenderNextTile();


private:
	Ui::RenderOptionsDialog *ui;
};

#endif // RENDEROPTIONSDIALOG_H
