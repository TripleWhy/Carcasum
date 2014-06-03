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
