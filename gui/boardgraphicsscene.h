#ifndef BOARDGRAPHICSSCENE_H
#define BOARDGRAPHICSSCENE_H

#include "core/game.h"
#include "jcz/jczutils.h"
#include "tileimagefactory.h"

#include <QWidget>
#include <QGraphicsScene>
#include <QSet>

#include <atomic>

#define DRAW_TILE_POSITION_TEXT 1

class BoardGraphicsScene : public QGraphicsScene
{
Q_OBJECT
public:
	static int const TILE_SIZE = 300;

private:
	Game * game;
	QList<QGraphicsItem *> tiles;
	QSet<QGraphicsRectItem *> openTiles;
	QGraphicsPixmapItem * placementTile;
	JCZUtils::TileFactory * tileFactory;
	TileImageFactory imgFactory;

	std::atomic<bool> running;
//	std::atomic<Move> userMove; // Does not compile and I have no idea why
	// workaround:
	std::atomic<bool> userMoveReady;
	Board::TilePlacement userMove;
	QList<Board::TilePlacement> const * possiblePlacements;

public:
	explicit BoardGraphicsScene(JCZUtils::TileFactory * tileFactory = 0, QObject * parent = 0);
	~BoardGraphicsScene();

	void setGame(Game * g);
	void setTileFactory(JCZUtils::TileFactory * factory);
	virtual Move getMove(Tile const * const tile, QList<Board::TilePlacement> const & placements, Game const * const game);

protected:
	virtual void mousePressEvent (QGraphicsSceneMouseEvent * mouseEvent);
	virtual void mouseMoveEvent(QGraphicsSceneMouseEvent * mouseEvent);

private:
	static void indexAt(QPointF scenePos, int & x, int & y);

private slots:
	void boardChanged(Board const * board);
};

#endif // BOARDGRAPHICSSCENE_H
