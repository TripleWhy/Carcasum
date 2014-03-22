#ifndef BOARDGRAPHICSSCENE_H
#define BOARDGRAPHICSSCENE_H

#include "static.h"
#include "core/game.h"
#include "core/player.h"
#include "jcz/tilefactory.h"
#include "tileimagefactory.h"

#include <QWidget>
#include <QGraphicsScene>
#include <QSet>
#include <QGraphicsSvgItem>

#include <atomic>
#include <mutex>

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
	TileImageFactory * imgFactory;
	QHash<QPoint, QGraphicsItemGroup *> possibleMeeplePlaces;
	TileMovesType possiblePlacements;

	std::mutex lock;
	int running;
	bool userMoveReady;
	TileMove userMove;
	MeepleMove userMeepleMove;

public:
	explicit BoardGraphicsScene(jcz::TileFactory * tileFactory = 0, TileImageFactory * imgFactory = 0, QObject * parent = 0);
	~BoardGraphicsScene();

	void setGame(const Game * const g);
	void setTileFactory(jcz::TileFactory * factory);
	void setTileImageFactory(TileImageFactory * factory);
	virtual TileMove getTileMove(int player, Tile const * const tile, MoveHistoryEntry const & move, TileMovesType const & placements, Game const * const game);
	virtual MeepleMove getMeepleMove(int player, Tile const * const tile, MoveHistoryEntry const & move, MeepleMovesType const & possible, Game const * const game);
	virtual void newGame(int player, Game const * const game);
	virtual void playerMoved(int player, Tile const * const tile, MoveHistoryEntry const & move, Game const * const game);
	virtual void endGame(Game const * const game);

protected:
	virtual void mousePressEvent (QGraphicsSceneMouseEvent * mouseEvent);
	virtual void mouseMoveEvent(QGraphicsSceneMouseEvent * mouseEvent);

private:
	static void indexAt(QPointF const & scenePos, uint & x, uint & y);
	QGraphicsItemGroup * meepleAt(QPointF const & scenePos);
	QGraphicsItemGroup * createMeeple(Node const * n, QPoint & point, const TileMove & tileMove, QColor const & color);
	void placeOpen();

private slots:
	void displayNewGame(int callDepth = 0);
	void displayPlayerMoved(void * data, int callDepth = 0);
	void displayGetTileMove(void * data, int callDepth = 0);
	void displayGetMeepleMove(void * data, int callDepth = 0);
private:
	struct DPMData
	{
		int player;
		Tile const * const tile;
		Move move;
	};
	struct DGTMData
	{
		Tile::TileSet tileSet;
		int tileType;
		TileMovesType placements;
	};
	struct DGMMData
	{
		int player;
		Tile const * const tile;
		MeepleMovesType possible;
		TileMove tileMove;
	};
};

#endif // BOARDGRAPHICSSCENE_H
