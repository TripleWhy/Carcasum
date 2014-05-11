#ifndef PLAYERINFOVIEW_H
#define PLAYERINFOVIEW_H

#include "static.h"
#include "tileimagefactory.h"
#include "guiIncludes.h"

namespace Ui {
class PlayerInfoView;
}

class PlayerInfoView : public QWidget
{
Q_OBJECT

private:
	int playerIndex = -1;
	Game const * game = 0;
	TileImageFactory * imgFactory = 0;
	
	QLabel * meepleLabels[MEEPLE_COUNT];
	QPalette normPalette;
	QPalette hlPalette;
	
public:
	explicit PlayerInfoView(QWidget *parent = 0);
	explicit PlayerInfoView(int player, Game const * game, TileImageFactory * tif, QWidget *parent = 0);
	~PlayerInfoView();
	void setPlayer(int player, const Game * game, TileImageFactory * tif);
	void setPlayerName(QString const & name);

public slots:
	void updateView();
	void displayTile(int player, int tileType);

private:
	Ui::PlayerInfoView *ui;
};

#endif // PLAYERINFOVIEW_H
