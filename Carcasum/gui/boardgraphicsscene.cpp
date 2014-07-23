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

#include "boardgraphicsscene.h"
#include "core/util.h"
#include "core/board.h"
#include <QGraphicsPixmapItem>
#include <QGraphicsSceneMouseEvent>
#include <QGraphicsColorizeEffect>

BoardGraphicsScene::BoardGraphicsScene(jcz::TileFactory * tileFactory, TileImageFactory * imgFactory, QObject * parent)
	: QGraphicsScene(parent),
	  game(0),
	  tileFactory(tileFactory),
	  imgFactory(imgFactory),
	  running(0),
	  userMoveReady(false)
{
	setBackgroundBrush(QBrush(QPixmap(":/jcz/sysimages/panel_bg.png")));

	tileLayer = new QGraphicsItemGroup();
	openLayer = new QGraphicsItemGroup();
	meepleLayer = new QGraphicsItemGroup();
	placementLayer = new QGraphicsItemGroup();
	meeplePlacementLayer = new QGraphicsItemGroup();
	addItem(tileLayer);
	addItem(openLayer);
	addItem(meepleLayer);
	addItem(placementLayer);
	addItem(meeplePlacementLayer);

	openLayer->setOpacity(0.1);
	placementLayer->setOpacity(0.7);
	
#if DRAW_TILE_POSITION_TEXT || DRAW_NODE_ID_TEXT
	textOverlayLayer = new QGraphicsItemGroup();
	addItem(textOverlayLayer);
	textOverlayLayer->setOpacity(0.8);
#endif
#if DRAW_NODE_ID_TEXT && DEBUG_IDS
	textNodeOverlayLayer = new QGraphicsItemGroup();
	addItem(textNodeOverlayLayer);
	textNodeOverlayLayer->setOpacity(0.8);
#endif

	placementTile = new QGraphicsPixmapItem();
	placementTile->setTransformationMode(Qt::SmoothTransformation);
	placementTile->setShapeMode(QGraphicsPixmapItem::BoundingRectShape);
	placementTile->setOffset(-BOARD_TILE_SIZE / 2.0, -BOARD_TILE_SIZE / 2.0);
}

BoardGraphicsScene::~BoardGraphicsScene()
{
	delete placementTile;
}

void BoardGraphicsScene::setGame(const Game * g)
{
	game = g;
}

void BoardGraphicsScene::setTileFactory(jcz::TileFactory * factory)
{
	tileFactory = factory;
}

void BoardGraphicsScene::setTileImageFactory(TileImageFactory * factory)
{
	imgFactory = factory;
}

TileMove BoardGraphicsScene::getTileMove(int /*player*/, const Tile * tile, const MoveHistoryEntry & /*move*/, TileMovesType const & placements)
{
	if (_quit)
		return TileMove();

	std::unique_lock<std::mutex> lck(lock);
	while (running == -1)
	{
		lck.unlock();
		Util::sleep(100);
		lck.lock();
	}
	if (running != 0)
		return TileMove();
	running = 1;
	lck.unlock();
	
	auto data = new DGTMData { tile->tileType, placements };
	displayGetTileMove(data);

	TileMove m;
	while (!_quit)
	{
		auto * thread = Util::InterruptableThread::currentInterruptableThread();
		if (thread != 0 && thread->isInterrupted())
		{
			break;
		}

		lck.lock();
		if (userMoveReady)
		{
			m = userMove;
			userMoveReady = false;

			if (std::find (placements.cbegin(), placements.cend(), m) != placements.cend())
				break;
		}
		else
		{
			lck.unlock();
			if (Util::isGUIThread())
			{
				QCoreApplication::processEvents();
				Util::sleep(10);
			}
			else
				Util::sleep(200);
		}
	}

	running = 0;
	return m;
}

void BoardGraphicsScene::displayGetTileMove(void * data, int callDepth)
{
	DGTMData * d = static_cast<DGTMData *>(data);
	
	if (!Util::isGUIThread())
	{
		Q_ASSERT(callDepth < 1);
		if (callDepth < 2)
		{
			QMetaObject::invokeMethod(this, "displayGetTileMove", Qt::QueuedConnection,
			                          Q_ARG(void *, data),
			                          Q_ARG(int, callDepth+1));
		}
		else
		{
			delete d;
		}
		return;
	}
	
	possiblePlacements = d->placements;
	placementTile->setPixmap(imgFactory->getImage(d->tileType));
	placementLayer->addToGroup(placementTile);
	for(auto it = openTiles.begin(); it != openTiles.end(); )
	{
		QGraphicsRectItem * rect = *it;
		uint const x = rect->data(0).toUInt();
		uint const y = rect->data(1).toUInt();
		rect->setBrush(Qt::darkGray);
		
		bool possible = false;
		for (TileMove const & p : d->placements)
		{
			if (p.x == x && p.y == y)
			{
				rect->setBrush(Qt::green);
				possible = true;
				break;
			}
		}
		if (possible)
			++it;
		else
			it = openTiles.erase(it);
	}
	
	delete d;
}

MeepleMove BoardGraphicsScene::getMeepleMove(int player, const Tile * tile, const MoveHistoryEntry & /*move*/, MeepleMovesType const & possible)
{
	if (_quit)
		return MeepleMove();

	std::unique_lock<std::mutex> lck(lock);
	while (running == -1)
	{
		lck.unlock();
		Util::sleep(100);
		lck.lock();
	}
	if (running != 0)
		return MeepleMove();
	running = 2;

	TileMove tileMove = userMove;
	lck.unlock();
	
	DGMMData * data = new DGMMData { player, tile, possible, tileMove };
	displayGetMeepleMove(data);

	MeepleMove m;
	while (!_quit)
	{
		auto * thread = Util::InterruptableThread::currentInterruptableThread();
		if (thread != 0 && thread->isInterrupted())
		{
			break;
		}

		lck.lock();
		if (userMoveReady)
		{
			m = userMeepleMove;
			userMoveReady = false;
			break;
		}
		else
		{
			lck.unlock();
			if (Util::isGUIThread())
			{
				QCoreApplication::processEvents();
				Util::sleep(20);
			}
			else
				Util::sleep(200);
		}
	}

	running = 0;
	return m;
}

void BoardGraphicsScene::displayGetMeepleMove(void * data, int callDepth)
{
	DGMMData * d = static_cast<DGMMData *>(data);
	if (!Util::isGUIThread())
	{
		Q_ASSERT(callDepth < 1);
		if (callDepth < 2)
		{
			QMetaObject::invokeMethod(this, "displayGetMeepleMove", Qt::QueuedConnection,
			                          Q_ARG(void *, data),
			                          Q_ARG(int, callDepth+1));
		}
		else
		{
			delete d;
		}
		return;
	}
	
	placementTile->setPos(d->tileMove.x * BOARD_TILE_SIZE, d->tileMove.y * BOARD_TILE_SIZE);

	QMap<uchar, QPoint> const & pointMap = imgFactory->getPoints(d->tile);
	QHash<QPoint, QGraphicsItemGroup *> points;
	for (MeepleMove const & p : d->possible)
	{
		if (p.isNull())
			continue;

		Q_ASSERT(pointMap.contains(p.nodeIndex));
		QPoint point = pointMap[p.nodeIndex];
		QGraphicsItemGroup * svg = createMeeple(d->tile->getCNodes()[p.nodeIndex], point, d->tileMove, imgFactory->getPlayerColor(d->player));
		svg->setData(0, qVariantFromValue(p.nodeIndex));
		svg->setOpacity(0.7);
		meeplePlacementLayer->addToGroup(svg);
		points.insert(point, svg);
	}
	possibleMeeplePlaces = points;
	
	delete d;
}

void BoardGraphicsScene::displayEndGame(int callDepth)
{
	if (!Util::isGUIThread())
	{
		if (callDepth < 2)
		{
			QMetaObject::invokeMethod(this, "displayEndGame", Qt::QueuedConnection,
			                          Q_ARG(int, callDepth+1));
		}
		return;
	}
	qDeleteAll(openLayer->childItems());
	openTiles.clear();
}

void BoardGraphicsScene::newGame(int /*player*/, const Game * game)
{
	setGame(game);
	displayNewGame();
}

void BoardGraphicsScene::displayNewGame(int callDepth)
{
	if (!Util::isGUIThread())
	{
		Q_ASSERT(callDepth < 1);
		if (callDepth < 2)
		{
			QMetaObject::invokeMethod(this, "displayNewGame", Qt::QueuedConnection,
			                          Q_ARG(int, callDepth+1));
		}
		return;
	}

	for (QGraphicsRectItem * frame : frames)
		delete frame;
	frames.clear();

	qDeleteAll(tileLayer->childItems());
	qDeleteAll(meepleLayer->childItems());
	qDeleteAll(meeplePlacementLayer->childItems());
#if DRAW_TILE_POSITION_TEXT || DRAW_NODE_ID_TEXT
	qDeleteAll(textOverlayLayer->childItems());
#endif

	if (placementTile->parentItem() != 0)
		removeItem(placementTile);

	const Board * board = game->getBoard();
	uint arraySize = board->getInternalSize();
	setSceneRect(-BOARD_TILE_SIZE / 2.0, -BOARD_TILE_SIZE / 2.0, arraySize * BOARD_TILE_SIZE, arraySize * BOARD_TILE_SIZE);

	for (uint i = 0; i < game->getPlayerCount(); ++i)
	{
		QGraphicsRectItem * rect = new QGraphicsRectItem(-BOARD_TILE_SIZE / 2.0, -BOARD_TILE_SIZE / 2.0, BOARD_TILE_SIZE, BOARD_TILE_SIZE);
		rect->setBrush(QBrush());
		rect->setPen(QPen(imgFactory->getPlayerColor(i), BOARD_TILE_FRAME_WIDTH, Qt::SolidLine, Qt::FlatCap, Qt::RoundJoin));
		rect->setOpacity(0.6);
		frames.push_back(rect);
	}

	for (uint y = 0; y < arraySize; ++y)
	{
		for (uint x = 0; x < arraySize; ++x)
		{
			Tile const * t = board->getTile(x, y);
			
#if DRAW_TILE_POSITION_TEXT
			{
				QGraphicsTextItem * text = new QGraphicsTextItem(QString("%1|%2").arg(x).arg(y));
				text->setFont(QFont("sans", 50));
				text->setDefaultTextColor(Qt::red);
				//			text->setPos(-TILE_SIZE / 2.0, -90 / 2.0);
				text->setPos(x * BOARD_TILE_SIZE -BOARD_TILE_SIZE / 2.0, y * BOARD_TILE_SIZE -BOARD_TILE_SIZE / 2.0);
				textOverlayLayer->addToGroup(text);
			}
#endif
			
			if (t == 0)
				continue;

#if DRAW_NODE_ID_TEXT && DEBUG_IDS
			{
				QGraphicsTextItem * text = new QGraphicsTextItem(QString("%1").arg(t->id));
				text->setFont(QFont("sans", 50));
				text->setDefaultTextColor(Qt::yellow);
				text->setPos(x * BOARD_TILE_SIZE -BOARD_TILE_SIZE / 2.0,
							 y * BOARD_TILE_SIZE +BOARD_TILE_SIZE / 4.0);
				textOverlayLayer->addToGroup(text);
			}
			for (uchar i = 0; i < t->getNodeCount(); ++i)
			{
				QPoint meeplePoint = imgFactory->getPoints(t)[i];
				mapMeeplePoint(meeplePoint, TileMove(x, y, t->orientation));
				QGraphicsTextItem * text = new QGraphicsTextItem(QString("%1(%2)").arg(t->getCNodes()[i]->id()).arg(t->getCNodes()[i]->realId()));
				text->setFont(QFont("sans", 25));
				text->setDefaultTextColor(Qt::magenta);
				text->setPos(meeplePoint.x(), meeplePoint.y());
				textOverlayLayer->addToGroup(text);
			}
#endif
			QPixmap const & img = imgFactory->getImage(t);
			QGraphicsPixmapItem * item = new QGraphicsPixmapItem(img);
			item->setTransformationMode(Qt::SmoothTransformation);
			item->setShapeMode(QGraphicsPixmapItem::BoundingRectShape);
			item->setOffset(-BOARD_TILE_SIZE / 2.0, -BOARD_TILE_SIZE / 2.0);
			item->setPos(x * BOARD_TILE_SIZE, y * BOARD_TILE_SIZE);
			item->setRotation(t->orientation * 90);

			tileLayer->addToGroup(item);
		}
	}

	placeOpen();
}

void BoardGraphicsScene::playerMoved(int player, const Tile * const tile, const MoveHistoryEntry & move)
{
	Q_ASSERT(tileFactory != 0);

	DPMData * callData = new DPMData
	{
	        player,
	        tile, move.move,
#if DRAW_NODE_ID_TEXT && DEBUG_IDS
	        game->getBoard()->getTiles(),
	        game->getMoveHistory()
#endif
	};
	displayPlayerMoved(callData);
}

void BoardGraphicsScene::undoneMove(const MoveHistoryEntry & move)
{
	Q_ASSERT(tileFactory != 0);

	std::unique_lock<std::mutex> lck(lock);
	running = -1;
	lck.unlock();

	if (move.move.tileMove.isNull())
		return;

	DUMData * callData = new DUMData { move };
	displayUndoneMove(callData);
}

void BoardGraphicsScene::endGame()
{
	displayEndGame();
}

Player *BoardGraphicsScene::clone() const
{
	return 0;
}

void BoardGraphicsScene::setRenderOpenTiles(bool render)
{
	renderOpenTiles = render;
	placeOpen();
}

void BoardGraphicsScene::setRenderFrames(bool render)
{
	renderFrames = render;
	if (!render)
	{
		for (auto * frame : frames)
			tileLayer->removeFromGroup(frame);
	}
}

void BoardGraphicsScene::displayPlayerMoved(void * data, int callDepth)
{
	DPMData * d = static_cast<DPMData *>(data);
	// Execute update in GUI thread only.
	if (!Util::isGUIThread())
	{
		Q_ASSERT(callDepth < 1);
		if (callDepth < 2)
		{
			QMetaObject::invokeMethod(this, "displayPlayerMoved", Qt::QueuedConnection,
			                          Q_ARG(void *, data),
			                          Q_ARG(int, callDepth+1));
		}
		else
		{
			delete d;
		}
		return;
	}

	if (items().contains(placementTile))
		removeItem(placementTile);
	qDeleteAll(meeplePlacementLayer->childItems());

	TileMove const & tileMove = d->move.tileMove;
	MeepleMove const & meepleMove = d->move.meepleMove;
	
#if DRAW_NODE_ID_TEXT && DEBUG_IDS
	qDeleteAll(textNodeOverlayLayer->childItems());
	{
		QGraphicsTextItem * text = new QGraphicsTextItem(QString("%1").arg(d->tile->id));
		text->setFont(QFont("sans", 50));
		text->setDefaultTextColor(Qt::yellow);
		text->setPos(tileMove.x * BOARD_TILE_SIZE - BOARD_TILE_SIZE / 2.0,
					 tileMove.y * BOARD_TILE_SIZE + BOARD_TILE_SIZE / 4.0);
		textOverlayLayer->addToGroup(text);
	}
	int tileOffset = (int)d->tiles.size() - (int)d->history.size();
	for (uint j = 0; j < d->history.size(); ++j)
	{
		Tile const * t = d->tiles[j + tileOffset];
		TileMove const & m = d->history[j].move.tileMove;
		for (uchar i = 0; i < t->getNodeCount(); ++i)
		{
			QPoint meeplePoint = imgFactory->getPoints(t)[i];
			mapMeeplePoint(meeplePoint, m);
			QGraphicsTextItem * text = new QGraphicsTextItem(QString("%1(%2)").arg(t->getCNodes()[i]->id()).arg(t->getCNodes()[i]->realId()));
			text->setFont(QFont("sans", 25));
			text->setDefaultTextColor(Qt::magenta);
			text->setPos(meeplePoint.x(), meeplePoint.y());
			textNodeOverlayLayer->addToGroup(text);
		}
	}
#endif
	QPixmap const & img = imgFactory->getImage(d->tile);
	QGraphicsPixmapItem * item = new QGraphicsPixmapItem(img);
	item->setTransformationMode(Qt::SmoothTransformation);
	item->setShapeMode(QGraphicsPixmapItem::BoundingRectShape);
	item->setOffset(-BOARD_TILE_SIZE / 2.0, -BOARD_TILE_SIZE / 2.0);
	item->setPos(tileMove.x * BOARD_TILE_SIZE, tileMove.y * BOARD_TILE_SIZE);
	item->setRotation(tileMove.orientation * 90);
	tileLayer->addToGroup(item);

	QGraphicsRectItem * frame = frames[d->player];
	frame->setPos(item->pos());
	tileLayer->removeFromGroup(frame);
	if (renderFrames)
		tileLayer->addToGroup(frame);

	if (!meepleMove.isNull())
	{
		QPoint meeplePoint = imgFactory->getPoints(d->tile)[meepleMove.nodeIndex];
		auto meeple = createMeeple(d->tile->getCNodes()[meepleMove.nodeIndex], meeplePoint, tileMove, imgFactory->getPlayerColor(d->player));
		meepleLayer->addToGroup(meeple);
//		meeple->ensureVisible();
	}

	{
		auto const & history = game->getMoveHistory();
		auto const & children = meepleLayer->childItems();
		Board const * b = game->getBoard();
		int j = 0;
		for (uint i = 0; i < history.size(); ++i)
		{
			Move const & m = history[i].move;
			if (m.tileMove.isNull() || m.meepleMove.isNull())
				continue;
			if (j >= children.length())	// This runs in gui thread so game thread can have added a move in between.
				break;

			Tile const * t = b->getTile(m.tileMove.x, m.tileMove.y);
			Node const * n = t->getNode(m.meepleMove.nodeIndex);
			if (n->getScored() != NotScored)
			{
				QGraphicsItemGroup * grp = static_cast<QGraphicsItemGroup *>(children[j]);
				QGraphicsSvgItem * fill = static_cast<QGraphicsSvgItem *>(grp->childItems()[0]);
				if (grp->graphicsEffect() == 0)
				{
					auto effect = new QGraphicsOpacityEffect(fill);
					effect->setOpacity(0.4);
					grp->setGraphicsEffect(effect);
				}
			}

			++j;
		}
	}

	placeOpen();
	
	delete d;
}

void BoardGraphicsScene::displayUndoneMove(void * data, int callDepth)
{
	DUMData * d = static_cast<DUMData *>(data);
	// Execute update in GUI thread only.
	if (!Util::isGUIThread())
	{
		Q_ASSERT(callDepth < 1);
		if (callDepth < 2)
		{
			QMetaObject::invokeMethod(this, "displayUndoneMove", Qt::QueuedConnection,
			                          Q_ARG(void *, data),
			                          Q_ARG(int, callDepth+1));
		}
		else
		{
			delete d;
		}
		return;
	}


	if (items().contains(placementTile))
		removeItem(placementTile);
	qDeleteAll(meeplePlacementLayer->childItems());

	for (auto frame = dynamic_cast<QGraphicsRectItem *>(tileLayer->childItems().last());
	     frames.contains(frame);
	     frame = dynamic_cast<QGraphicsRectItem *>(tileLayer->childItems().last()))
	{
//		tileLayer->removeFromGroup(frame);	//Apparently does not do what I want. Also sets frame's parents to 0, so it looks correct at first. Qt bug?
		removeItem(frame);
	}
	delete tileLayer->childItems().last();

	if (!d->move.move.meepleMove.isNull())
		delete meepleLayer->childItems().last();

	placeOpen();

	std::unique_lock<std::mutex> lck(lock);
	running = 0;
	lck.unlock();

	delete d;
}

void BoardGraphicsScene::mousePressEvent(QGraphicsSceneMouseEvent * mouseEvent)
{
	std::lock_guard<std::mutex> guard(lock);
	int r = running;
	if (r == 1)
	{
		uint x, y;
		indexAt(mouseEvent->scenePos(), x, y);
		QGraphicsRectItem * item = 0;
		for (QGraphicsRectItem * i : openTiles)
		{
			if (i->data(0).toUInt() == x && i->data(1).toUInt() == y)
			{
				item = i;
				break;
			}
		}

		if (mouseEvent->buttons() == Qt::LeftButton)
		{
			if (item == 0)
				return;

			userMove.x = x;
			userMove.y = y;
			userMove.orientation = (Tile::Side)int(placementTile->rotation() / 90);
			userMoveReady = true;
		}
		else if (mouseEvent->buttons() == Qt::RightButton)
		{
			int orientation = int(placementTile->rotation() / 90);
			if (item != 0)
			{
				TileMovesType const & placements = possiblePlacements;
#if defined(QT_NO_DEBUG) && !defined(QT_FORCE_ASSERTS)
				for (int i = 0; i < 4; ++i)
				{
#else
				for (int i = 0; i < 5; ++i)
				{
					Q_ASSERT(i < 4);
#endif
					orientation = (orientation + 1) % 4;
					for (TileMove const & p : placements)
						if (p.x == x && p.y == y && p.orientation == orientation)
							goto hell;
				}
				hell:;
			}
			else
			{
				orientation = (orientation + 1) % 4;
			}
			placementTile->setRotation(orientation * 90);
		}
	}
	else if (r == 2)
	{
		if (mouseEvent->buttons() == Qt::LeftButton)
		{
			QGraphicsItemGroup * m = meepleAt(mouseEvent->scenePos());
			if (m == 0)
				return;
			userMeepleMove.nodeIndex = m->data(0).value<uchar>();
			userMoveReady = true;
		}
		else if (mouseEvent->buttons() == Qt::RightButton)
		{
			userMeepleMove = -1;
			userMoveReady = true;
		}
	}
}

void BoardGraphicsScene::mouseMoveEvent(QGraphicsSceneMouseEvent * mouseEvent)
{
	std::lock_guard<std::mutex> guard(lock);
	int r = running;
	if (r == 0)
	{
		placementTile->setPos(mouseEvent->scenePos());
	}
	else if (r == 1)
	{
		uint x, y;
		indexAt(mouseEvent->scenePos(), x, y);
		QGraphicsRectItem * item = 0;
		for (QGraphicsRectItem * i : openTiles)
		{
			if (i->data(0).toUInt() == x && i->data(1).toUInt() == y)
			{
				item = i;
				break;
			}
		}

		if (item == 0)
		{
			placementTile->setPos(mouseEvent->scenePos());
		}
		else
		{
			placementTile->setPos(item->pos());
			int orientation = int(placementTile->rotation() / 90);
			TileMovesType const & placements = possiblePlacements;
//#if defined(QT_NO_DEBUG) && !defined(QT_FORCE_ASSERTS)
			for (int i = 0; i < 4; ++i)
			{
//#else
//			for (int i = 0; i < 5; ++i)
//			{
//				Q_ASSERT(i < 4);
//#endif
				for (TileMove const & p : placements)
					if (p.x == x && p.y == y && p.orientation == orientation)
						goto hell;
				orientation = (orientation + 1) % 4;
			}
			hell:
			placementTile->setRotation(orientation * 90);
		}
	}
	else if (r == 2)
	{
		QGraphicsItemGroup * m = meepleAt(mouseEvent->scenePos());
		for (QGraphicsItemGroup * g : possibleMeeplePlaces.values())
		{
			if (g == m)
				g->setOpacity(0.9);
			else
				g->setOpacity(0.7);
		}
	}
}

void BoardGraphicsScene::indexAt(const QPointF & scenePos, uint & x, uint & y)
{
	x = ((int)scenePos.x() + BOARD_TILE_SIZE / 2) / BOARD_TILE_SIZE;
	y = ((int)scenePos.y() + BOARD_TILE_SIZE / 2) / BOARD_TILE_SIZE;
}

QGraphicsItemGroup * BoardGraphicsScene::meepleAt(const QPointF & scenePos)
{
	QPoint const & mousePoint = scenePos.toPoint();
	static int const maxSqDist = 75*75;
	int bestSqDist = maxSqDist;
	QPoint best;
	for (QPoint const & point : possibleMeeplePlaces.keys())
	{
		int dx = mousePoint.x() - point.x();
		int dy = mousePoint.y() - point.y();
		int sqDist = dx * dx + dy * dy;
		if (sqDist < maxSqDist && sqDist < bestSqDist)
		{
			bestSqDist = sqDist;
			best = point;
		}
	}
	if (bestSqDist < maxSqDist)
		return possibleMeeplePlaces[best];
	else
		return 0;
}

void BoardGraphicsScene::mapMeeplePoint(QPoint & point, TileMove const & tileMove)
{
	QTransform t;
	t.translate(BOARD_TILE_SIZE / 2.0, BOARD_TILE_SIZE / 2.0);
	t.rotate(tileMove.orientation * 90);
	t.translate(-BOARD_TILE_SIZE / 2.0, -BOARD_TILE_SIZE / 2.0);
	point = t.map(point);

	point.rx() += tileMove.x * BOARD_TILE_SIZE - BOARD_TILE_SIZE / 2;
	point.ry() += tileMove.y * BOARD_TILE_SIZE - BOARD_TILE_SIZE / 2;
}

QGraphicsItemGroup *BoardGraphicsScene::createMeeple(Node const * n, QPoint & point, TileMove const & tileMove, QColor const & color)
{
	mapMeeplePoint(point, tileMove);

	QGraphicsItemGroup * svg = new QGraphicsItemGroup();

	QGraphicsSvgItem * fill = new QGraphicsSvgItem(imgFactory->getMeepleFillSvg(n), svg);
	QGraphicsSvgItem * outline = new QGraphicsSvgItem(imgFactory->getMeepleOutlineSvg(n), svg);
	auto colorEffect = new QGraphicsColorizeEffect(fill);
	colorEffect->setColor(color);
	fill->setGraphicsEffect(colorEffect);
	svg->addToGroup(fill);
	svg->addToGroup(outline);

	auto size = svg->boundingRect().size();
	qreal max = qMax(size.width(), size.height());
	qreal scale = BOARD_MEEPLE_SIZE / max;
	svg->setScale( scale );
	size *= scale;

	QPointF pos(point.x() - size.width()  / 2,
				point.y() - size.height() / 2);
	svg->setPos(pos);

	return svg;
}

void BoardGraphicsScene::placeOpen()
{
	qDeleteAll(openLayer->childItems());
	openTiles.clear();

	if (!renderOpenTiles)
		return;

	const Board * board = game->getBoard();
	QList<QPoint> const & openPlaces = board->getOpenPlaces();
	for (QPoint const & open : openPlaces)
	{
		uint x = open.x();
		uint y = open.y();

		QGraphicsRectItem * rect = new QGraphicsRectItem(-BOARD_TILE_SIZE / 2.0, -BOARD_TILE_SIZE / 2.0, BOARD_TILE_SIZE, BOARD_TILE_SIZE);
		rect->setPos(x * BOARD_TILE_SIZE, y * BOARD_TILE_SIZE);
		rect->setBrush(Qt::lightGray);
		rect->setData(0, x);
		rect->setData(1, y);

		openLayer->addToGroup(rect);
		openTiles.append(rect);
	}
}
