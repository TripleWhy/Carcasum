#include "boardgraphicsscene.h"

#include <QGraphicsPixmapItem>
#include <QGraphicsSceneMouseEvent>
#include <QGraphicsColorizeEffect>

BoardGraphicsScene::BoardGraphicsScene(jcz::TileFactory * tileFactory, QObject * parent)
	: QGraphicsScene(parent),
	  game(0),
	  tileFactory(tileFactory),
	  imgFactory(tileFactory),
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

	openLayer->setOpacity(0.2);
	placementLayer->setOpacity(0.7);
	
#if DRAW_TILE_POSITION_TEXT
	textOverlayLayer = new QGraphicsItemGroup();
	addItem(textOverlayLayer);
	textOverlayLayer->setOpacity(0.2);
#endif

	placementTile = new QGraphicsPixmapItem();
	placementTile->setTransformationMode(Qt::SmoothTransformation);
	placementTile->setShapeMode(QGraphicsPixmapItem::BoundingRectShape);
	placementTile->setOffset(-TILE_SIZE / 2.0, -TILE_SIZE / 2.0);
}

BoardGraphicsScene::~BoardGraphicsScene()
{
	delete placementTile;
}

void BoardGraphicsScene::setGame(Game const *const g)
{
	game = g;
}

void BoardGraphicsScene::setTileFactory(jcz::TileFactory * factory)
{
	tileFactory = factory;
}

TileMove BoardGraphicsScene::getTileMove(int /*player*/, Tile const * const tile, QList<TileMove> const & placements, Game const * const game)
{
	std::unique_lock<std::mutex> lck(lock);
	if (running != 0)
		return TileMove();
	running = 1;
	lck.unlock();
	
	auto data = new DGTMData { tile->tileSet, tile->tileType, placements };
	displayGetTileMove(data);

	TileMove m;
	while (true)
	{
		lck.lock();
		if (userMoveReady)
		{
			m = userMove;
			userMoveReady = false;

			if (placements.contains(m))
				break;
		}
		else
		{
			lck.unlock();
			if (Util::isGUIThread())
			{
				qDebug() << "GUI thread";
				QCoreApplication::processEvents();
			}
			QThread::currentThread()->yieldCurrentThread();
		}
	}

	running = 0;
	lck.unlock();
	return TileMove(m.x, m.y, m.orientation);
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
	placementTile->setPixmap(imgFactory.getImage(d->tileSet, d->tileType));
	placementLayer->addToGroup(placementTile);
	for(auto it = openTiles.begin(); it != openTiles.end(); )
	{
		QGraphicsRectItem * rect = *it;
		uint const x = rect->data(0).toUInt();
		uint const y = rect->data(1).toUInt();
		rect->setBrush(Qt::black);
		
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

MeepleMove BoardGraphicsScene::getMeepleMove(int player, Tile const * const tile, QVarLengthArray<MeepleMove, NODE_ARRAY_LENGTH> const & possible, Game const * const game)
{
	std::unique_lock<std::mutex> lck(lock);
	if (running != 0)
		return 0;
	running = 2;

	TileMove tileMove = userMove;
	lck.unlock();
	
	DGMMData * data = new DGMMData { player, tile, possible, tileMove };
	displayGetMeepleMove(data);

	MeepleMove m = 0;
	while (true)
	{
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
			QThread::currentThread()->yieldCurrentThread();
		}
	}

	running = 0;
	lck.unlock();
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
	
	placementTile->setPos(d->tileMove.x * TILE_SIZE, d->tileMove.y * TILE_SIZE);

	QMap<const Node *, QPoint> pointMap = imgFactory.getPoints(d->tile);
	QHash<QPoint, QGraphicsItemGroup *> points;
	for (MeepleMove const & p : d->possible)
	{
		if (p == 0)
			continue;

		Q_ASSERT(pointMap.contains(p));
		QPoint point = pointMap[p];
		QGraphicsItemGroup * svg = createMeeple(p, point, d->tileMove, imgFactory.getPlayerColor(d->player));
		svg->setData(0, qVariantFromValue((void *)p));
		svg->setOpacity(0.7);
		meeplePlacementLayer->addToGroup(svg);
		points.insert(point, svg);
	}
	possibleMeeplePlaces = points;
	
	delete d;
}

void BoardGraphicsScene::newGame(int /*player*/, const Game * const game)
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
	qDeleteAll(tileLayer->childItems());
	qDeleteAll(openLayer->childItems());
	qDeleteAll(meepleLayer->childItems());
	qDeleteAll(meeplePlacementLayer->childItems());
#if DRAW_TILE_POSITION_TEXT
	qDeleteAll(textOverlayLayer->childItems());
#endif

	openTiles.clear();
	removeItem(placementTile);

	const Board * board = game->getBoard();
	uint arraySize = board->getInternalSize();
	setSceneRect(-TILE_SIZE / 2.0, -TILE_SIZE / 2.0, arraySize * TILE_SIZE, arraySize * TILE_SIZE);

	for (uint y = 0; y < arraySize; ++y)
	{
		for (uint x = 0; x < arraySize; ++x)
		{
			Tile const * t = board->getTile(x, y);
			
#if DRAW_TILE_POSITION_TEXT
			QGraphicsTextItem * text = new QGraphicsTextItem(QString("%1|%2").arg(x).arg(y));
			text->setFont(QFont("sans", 50));
			text->setDefaultTextColor(Qt::red);
//			text->setPos(-TILE_SIZE / 2.0, -90 / 2.0);
			text->setPos(x * TILE_SIZE -TILE_SIZE / 2.0, y * TILE_SIZE -TILE_SIZE / 2.0);
			textOverlayLayer->addToGroup(text);
#endif
			
			if (t == 0)
				continue;

			QPixmap const & img = imgFactory.getImage(t);
			QGraphicsPixmapItem * item = new QGraphicsPixmapItem(img);
			item->setTransformationMode(Qt::SmoothTransformation);
			item->setShapeMode(QGraphicsPixmapItem::BoundingRectShape);
			item->setOffset(-TILE_SIZE / 2.0, -TILE_SIZE / 2.0);
			item->setPos(x * TILE_SIZE, y * TILE_SIZE);
			item->setRotation(t->orientation * 90);

			tileLayer->addToGroup(item);
		}
	}

	placeOpen();
}

void BoardGraphicsScene::playerMoved(int player, Tile const * const tile, const TileMove & tileMove, const MeepleMove & meepleMove, const Game * const /*game*/)
{
	Q_ASSERT(tileFactory != 0);

	QPoint const & meeplePoint = imgFactory.getPoints(tile)[meepleMove];
	DPMData * callData = new DPMData { player, tile->tileSet, tile->tileType, tileMove, meepleMove, meeplePoint };
	displayPlayerMoved(callData);
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
	qDeleteAll(openLayer->childItems());
	openTiles.clear();
	removeItem(placementTile);
	qDeleteAll(meeplePlacementLayer->childItems());

	QPixmap const & img = imgFactory.getImage(d->tileSet, d->tileType);
	QGraphicsPixmapItem * item = new QGraphicsPixmapItem(img);
	item->setTransformationMode(Qt::SmoothTransformation);
	item->setShapeMode(QGraphicsPixmapItem::BoundingRectShape);
	item->setOffset(-TILE_SIZE / 2.0, -TILE_SIZE / 2.0);
	item->setPos(d->tileMove.x * TILE_SIZE, d->tileMove.y * TILE_SIZE);
	item->setRotation(d->tileMove.orientation * 90);
	tileLayer->addToGroup(item);

	if (d->meepleMove != 0)
	{
		auto meeple = createMeeple(d->meepleMove, d->meeplePoint, d->tileMove, imgFactory.getPlayerColor(d->player));
		meepleLayer->addToGroup(meeple);
//		meeple->ensureVisible();
	}

	placeOpen();
	
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
			if (i->data(0).toInt() == x && i->data(1).toInt() == y)
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
				QList<TileMove> const & placements = possiblePlacements;
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
			userMeepleMove = (MeepleMove)m->data(0).value<void *>();
			userMoveReady = true;
		}
		else if (mouseEvent->buttons() == Qt::RightButton)
		{
			userMeepleMove = 0;
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
			QList<TileMove> const & placements = possiblePlacements;
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
	x = (scenePos.x() + TILE_SIZE / 2) / TILE_SIZE;
	y = (scenePos.y() + TILE_SIZE / 2) / TILE_SIZE;
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

QGraphicsItemGroup *BoardGraphicsScene::createMeeple(MeepleMove const & p, QPoint & point, TileMove const & tileMove, QColor const & color)
{
	QTransform t;
	t.translate(TILE_SIZE / 2.0, TILE_SIZE / 2.0);
	t.rotate(tileMove.orientation * 90);
	t.translate(-TILE_SIZE / 2.0, -TILE_SIZE / 2.0);
	point = t.map(point);

	point.rx() += tileMove.x * TILE_SIZE - TILE_SIZE / 2;
	point.ry() += tileMove.y * TILE_SIZE - TILE_SIZE / 2;

	QGraphicsItemGroup * svg = new QGraphicsItemGroup();

	QGraphicsSvgItem * fill = new QGraphicsSvgItem(imgFactory.getMeepleFillSvg(p), svg);
	QGraphicsSvgItem * outline = new QGraphicsSvgItem(imgFactory.getMeepleOutlineSvg(p), svg);
	auto colorEffect = new QGraphicsColorizeEffect(fill);
	colorEffect->setColor(color);
	fill->setGraphicsEffect(colorEffect);
	svg->addToGroup(fill);
	svg->addToGroup(outline);

	auto size = svg->boundingRect().size();
	qreal max = qMax(size.width(), size.height());
	qreal scale = MEEPLE_SIZE / max;
	svg->setScale( scale );
	size *= scale;

	QPointF pos(point.x() - size.width()  / 2,
				point.y() - size.height() / 2);
	svg->setPos(pos);

	return svg;
}

void BoardGraphicsScene::placeOpen()
{
	const Board * board = game->getBoard();
	QList<QPoint> const & openPlaces = board->getOpenPlaces();
	for (QPoint const & open : openPlaces)
	{
		uint x = open.x();
		uint y = open.y();

		QGraphicsRectItem * rect = new QGraphicsRectItem(-TILE_SIZE / 2.0, -TILE_SIZE / 2.0, TILE_SIZE, TILE_SIZE);
		rect->setPos(x * TILE_SIZE, y * TILE_SIZE);
		rect->setBrush(Qt::gray);
		rect->setData(0, x);
		rect->setData(1, y);

		openLayer->addToGroup(rect);
		openTiles.append(rect);
	}
}
