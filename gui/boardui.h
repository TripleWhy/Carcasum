#ifndef BOARDUI_H
#define BOARDUI_H

#include "core/game.h"
#include "tileui.h"

#include <QWidget>
#include <QGridLayout>

class BoardUI : public QWidget
{
Q_OBJECT

private:
	Game * game;
	int size;
	QList<TileUI *> tiles;

	TileUI * tmp;

public:
	explicit BoardUI(QWidget *parent = 0);
	~BoardUI();

	void setGame(Game * g);

private slots:
	void boardChanged(Board const * board);
};

#endif // BOARDUI_H
