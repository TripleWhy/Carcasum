#ifndef BOARDGRAPHICSSCENE_H
#define BOARDGRAPHICSSCENE_H

#include "core/game.h"
#include "jcz/jczutils.h"
#include "tileimagefactory.h"

#include <QWidget>
#include <QGraphicsScene>

#include <atomic>

class BoardGraphicsScene : public QGraphicsScene
{
Q_OBJECT
public:
	static int const TILE_SIZE = 300;

private:
	Game * game;
	QList<QGraphicsPixmapItem *> tiles;
//	QList<TileUI *> openTiles;
	JCZUtils::TileFactory * tileFactory;
	TileImageFactory imgFactory;

	std::atomic<bool> running;
//	std::atomic<Move> userMove; // Does not compile and I have no idea why
	// workaround:
	std::atomic<bool> userMoveReady;
	Board::TilePlacement userMove;

public:
	explicit BoardGraphicsScene(JCZUtils::TileFactory * tileFactory = 0, QObject * parent = 0);
	~BoardGraphicsScene();

	void setGame(Game * g);
	void setTileFactory(JCZUtils::TileFactory * factory);
	virtual Move getMove(Tile const * const tile, QList<Board::TilePlacement> const & placements, Game const * const game);

private slots:
	void boardChanged(Board const * board);
	void tilePlaced();
};

#endif // BOARDGRAPHICSSCENE_H
