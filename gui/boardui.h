#ifndef BOARDUI_H
#define BOARDUI_H

#include "core/game.h"
#include "tileui.h"
#include "jcz/jczutils.h"

#include <QWidget>
#include <QGridLayout>

#include <atomic>

class BoardUI : public QWidget
{
Q_OBJECT

private:
	Game * game;
	QList<TileUI *> tiles;
	QList<TileUI *> openTiles;
	JCZUtils::TileFactory * tileFactory;

	QSize size;

	std::atomic<bool> running;
//	std::atomic<Move> userMove; // Does not compile and I have no idea why
	// workaround:
	std::atomic<bool> userMoveReady;
	Board::TilePlacement userMove;

public:
	explicit BoardUI(JCZUtils::TileFactory * tileFactory = 0, QWidget *parent = 0);
	~BoardUI();

	virtual QSize sizeHint() const;

	void setGame(Game * g);
	void setTileFactory(JCZUtils::TileFactory * factory);
	virtual Move getMove(Tile const * const tile, QList<Board::TilePlacement> const & placements, Game const * const game);

private slots:
	void boardChanged(Board const * board);
	void tilePlaced();
};

#endif // BOARDUI_H
