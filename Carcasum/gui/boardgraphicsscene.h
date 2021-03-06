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

#ifndef BOARDGRAPHICSSCENE_H
#define BOARDGRAPHICSSCENE_H

#include "static.h"
#include "core/game.h"
#include "core/player.h"
#include "jcz/tilefactory.h"
#include "tileimagefactory.h"
#include "guiIncludes.h"
#include <atomic>
#include <mutex>

class BoardGraphicsScene : public QGraphicsScene, public Player
{
Q_OBJECT

private:
	Game const * game;

	QGraphicsItemGroup * tileLayer;
	QGraphicsItemGroup * openLayer;
	QGraphicsItemGroup * meepleLayer;
	QGraphicsItemGroup * placementLayer;
	QGraphicsItemGroup * meeplePlacementLayer;
#if DRAW_TILE_POSITION_TEXT || DRAW_NODE_ID_TEXT
	QGraphicsItemGroup * textOverlayLayer;
	QGraphicsItemGroup * textNodeOverlayLayer;
#endif

	QList<QGraphicsRectItem *> frames;
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
	bool _quit = false;

	bool renderOpenTiles = true;
	bool renderFrames = true;

public:
	explicit BoardGraphicsScene(jcz::TileFactory * tileFactory = 0, TileImageFactory * imgFactory = 0, QObject * parent = 0);
	~BoardGraphicsScene();

	void setGame(const Game * g);
	void setTileFactory(jcz::TileFactory * factory);
	void setTileImageFactory(TileImageFactory * factory);
	virtual TileMove getTileMove(int player, Tile const * tile, MoveHistoryEntry const & move, TileMovesType const & placements);
	virtual MeepleMove getMeepleMove(int player, Tile const * tile, MoveHistoryEntry const & move, MeepleMovesType const & possible);
	virtual void newGame(int player, Game const * game);
	virtual void playerMoved(int player, Tile const * const tile, MoveHistoryEntry const & move);
	virtual void undoneMove(MoveHistoryEntry const & move);
	virtual void endGame();
	virtual QString getTypeName() const { return "BoardGraphicsScene"; }
	virtual Player * clone() const;
	void quit() { _quit = true; }

	void setRenderOpenTiles(bool render);
	void setRenderFrames(bool render);

protected:
	virtual void mousePressEvent (QGraphicsSceneMouseEvent * mouseEvent);
	virtual void mouseMoveEvent(QGraphicsSceneMouseEvent * mouseEvent);

private:
	static void indexAt(QPointF const & scenePos, uint & x, uint & y);
	QGraphicsItemGroup * meepleAt(QPointF const & scenePos);
	void mapMeeplePoint(QPoint & point, const TileMove & tileMove);
	QGraphicsItemGroup * createMeeple(Node const * n, QPoint & point, const TileMove & tileMove, QColor const & color);
	void placeOpen();

private slots:
	void displayNewGame(int callDepth = 0);
	void displayPlayerMoved(void * data, int callDepth = 0);
	void displayUndoneMove(void * data, int callDepth = 0);
	void displayGetTileMove(void * data, int callDepth = 0);
	void displayGetMeepleMove(void * data, int callDepth = 0);
	void displayEndGame(int callDepth = 0);
private:
	struct DPMData
	{
		int player;
		Tile const * const tile;
		Move move;
#if DRAW_NODE_ID_TEXT && DEBUG_IDS
		std::vector<Tile const *> const tiles;
		std::vector<MoveHistoryEntry> const history;
#endif
	};
	struct DUMData
	{
		MoveHistoryEntry move;
	};
	struct DGTMData
	{
		TileTypeType tileType;
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
