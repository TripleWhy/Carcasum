#ifndef BOARDGRAPHICSSCENE_H
#define BOARDGRAPHICSSCENE_H

#include "core/game.h"
#include "core/player.h"
#include "jcz/tilefactory.h"
#include "tileimagefactory.h"

#include <QWidget>
#include <QGraphicsScene>
#include <QSet>
#include <QGraphicsSvgItem>

#include <atomic>

#define DRAW_TILE_POSITION_TEXT 1

class BoardGraphicsScene : public QGraphicsScene, public Player
{
Q_OBJECT
public:
	static int const TILE_SIZE = 300;
	static int const MEEPLE_SIZE = 75;

private:
	Game const * game;

	QGraphicsItemGroup * tileLayer;
	QGraphicsItemGroup * openLayer;
	QGraphicsItemGroup * meepleLayer;
	QGraphicsItemGroup * placementLayer;
	QGraphicsItemGroup * meeplePlacementLayer;
#if DRAW_TILE_POSITION_TEXT
	QGraphicsItemGroup * textOverlayLayer;
#endif

	QList<QGraphicsRectItem *> openTiles;
//	QList<QGraphicsSvgItem *> meepleOutlines;
	QGraphicsPixmapItem * placementTile;
	jcz::TileFactory * tileFactory;
	TileImageFactory imgFactory;
	QHash<QPoint, QGraphicsItemGroup *> possibleMeeplePlaces;

	std::atomic<int> running;
//	std::atomic<Move> userMove; // Does not compile and I have no idea why
	// workaround:
	std::atomic<bool> userMoveReady;
	TileMove userMove;
	MeepleMove userMeepleMove;
	QList<TileMove> const * possiblePlacements;
//	QMap<const Node *, QPoint> possibleMeeples;

public:
	explicit BoardGraphicsScene(jcz::TileFactory * tileFactory = 0, QObject * parent = 0);
	~BoardGraphicsScene();

	void setGame(const Game * const g);
	void setTileFactory(jcz::TileFactory * factory);
	virtual TileMove getTileMove(Tile const * const tile, QList<TileMove> const & placements, Game const * const game);
	virtual MeepleMove getMeepleMove(Tile const * const tile, QVarLengthArray<MeepleMove, NODE_ARRAY_LENGTH> const & possible, Game const * const game);
	virtual void newGame(Game const * const game);
	virtual void playerMoved(const Tile * const tile, TileMove const & tileMove, MeepleMove const & meepleMove, Game const * const game);

protected:
	virtual void mousePressEvent (QGraphicsSceneMouseEvent * mouseEvent);
	virtual void mouseMoveEvent(QGraphicsSceneMouseEvent * mouseEvent);

private:
	static void indexAt(QPointF const & scenePos, int & x, int & y);
	QGraphicsItemGroup * meepleAt(QPointF const & scenePos);
	QGraphicsItemGroup * createMeeple(const MeepleMove & p, QPoint & point, const TileMove & tileMove, QColor const & color);
	void placeOpen();

private:
	struct DPMData
	{
		int callDepth;
		Tile::TileSet tileSet;
		int tileType;
		TileMove tileMove;
		MeepleMove meepleMove;
		QPoint meeplePoint;
	};
private slots:
	void displayNewGame();
	void displayPlayerMoved(void * data);
};

#endif // BOARDGRAPHICSSCENE_H
