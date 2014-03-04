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

	placementTile = new QGraphicsPixmapItem();
	placementTile->setTransformationMode(Qt::SmoothTransformation);
	placementTile->setShapeMode(QGraphicsPixmapItem::BoundingRectShape);
	placementTile->setOffset(-TILE_SIZE / 2.0, -TILE_SIZE / 2.0);
}

BoardGraphicsScene::~BoardGraphicsScene()
{
	delete placementTile;
}

void BoardGraphicsScene::setGame(Game * g)
{
	if (game != 0)
	{
		disconnect(game, SIGNAL(boardChanged(const Board*)), this, SLOT(boardChanged(const Board*)));
	}

	game = g;

	connect(game, SIGNAL(boardChanged(const Board*)), this, SLOT(boardChanged(const Board*)));
}

void BoardGraphicsScene::setTileFactory(jcz::TileFactory * factory)
{
	tileFactory = factory;
}

TileMove BoardGraphicsScene::getMove(const Tile * const tile, const QList<Board::TilePlacement> & placements, const Game * const /*game*/)
{
	if (running.exchange(1))
		return TileMove();

	possiblePlacements = &placements;
	placementTile->setPixmap(imgFactory.getImage(tile));
	placementLayer->addToGroup(placementTile);
	for(auto it = openTiles.begin(); it != openTiles.end(); )
	{
		QGraphicsRectItem * rect = *it;
		uint const x = rect->data(0).toUInt();
		uint const y = rect->data(1).toUInt();
		rect->setBrush(Qt::black);

		bool possible = false;
		for (Board::TilePlacement const & p : placements)
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

	if (Util::isGUIThread())
	{
		qDebug() << "GUI thread";
		//TODO?
	}

	Board::TilePlacement m;
	while (true)
	{
		if (userMoveReady)
		{
			m = userMove;
			userMoveReady = false;

			if (placements.contains(m))
				break;
		}
		else
			QThread::currentThread()->yieldCurrentThread();
	}

	running = 0;
	return TileMove(m.x, m.y, m.orientation);
}

MeepleMove BoardGraphicsScene::getMeepleMove(const Tile * const tile, const QVarLengthArray<MeepleMove, NODE_ARRAY_LENGTH> & possible, const Game * const game)
{
	if (running.exchange(2)) // Not exactly correct anymore
		return 0;

	Board::TilePlacement tileMove = userMove;
	placementTile->setPos(tileMove.x * TILE_SIZE, tileMove.y * TILE_SIZE);

	QMap<const Node *, QPoint> pointMap = imgFactory.getPoints(tile);
	QHash<QPoint, QGraphicsItemGroup *> points;
	for (MeepleMove const & p : possible)
	{
		if (p == 0)
			continue;
		Q_ASSERT(pointMap.contains(p));
		QPoint point = pointMap[p];

		QTransform t;
		t.translate(TILE_SIZE / 2.0, TILE_SIZE / 2.0);
		t.rotate(tile->orientation * 90);
		t.translate(-TILE_SIZE / 2.0, -TILE_SIZE / 2.0);
		point = t.map(point);

		point.rx() += tileMove.x * TILE_SIZE - TILE_SIZE / 2;
		point.ry() += tileMove.y * TILE_SIZE - TILE_SIZE / 2;

		QGraphicsItemGroup * svg = new QGraphicsItemGroup();
		svg->setData(0, qVariantFromValue((void *)p));
		points.insert(point, svg);

		QGraphicsSvgItem * fill = new QGraphicsSvgItem(imgFactory.getMeepleFillSvg(p), svg);
		QGraphicsSvgItem * outline = new QGraphicsSvgItem(imgFactory.getMeepleOutlineSvg(p), svg);
		auto color = new QGraphicsColorizeEffect(fill);
		color->setColor(Qt::red);
		fill->setGraphicsEffect(color);
		svg->addToGroup(fill);
		svg->addToGroup(outline);
//		meepleOutlines.append(svg);
		svg->setOpacity(0.7);


		auto size = svg->boundingRect().size();
		qreal max = qMax(size.width(), size.height());
		qreal scale = MEEPLE_SIZE / max;
		svg->setScale( scale );
		size *= scale;

		QPointF pos(point.x() - size.width()  / 2,
					point.y() - size.height() / 2);
		svg->setPos(pos);

		qDebug() << tile->orientation << p->t << point << pos << size;

		meeplePlacementLayer->addToGroup(svg);

		addRect(point.x(), point.y(), 5, 5, QPen(), Qt::red);

	}
	possibleMeeplePlaces = points;

	MeepleMove m = 0;
	while (true)
	{
		if (userMoveReady)
		{
			m = userMeepleMove;
			userMoveReady = false;
			break;
		}
		else
			QThread::currentThread()->yieldCurrentThread();
	}

	running = 0;
	return m;
}

void BoardGraphicsScene::mousePressEvent(QGraphicsSceneMouseEvent * mouseEvent)
{
	int r = running;
	if (r == 1)
	{
		int x, y;
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
				QList<Board::TilePlacement> const & placements = *possiblePlacements;
#if defined(QT_NO_DEBUG) && !defined(QT_FORCE_ASSERTS)
				for (int i = 0; i < 4; ++i)
				{
#else
				for (int i = 0; i < 5; ++i)
				{
					Q_ASSERT(i < 4);
#endif
					orientation = (orientation + 1) % 4;
					for (Board::TilePlacement const & p : placements)
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
		QGraphicsItemGroup * m = meepleAt(mouseEvent->scenePos());
		if (m == 0)
			return;
		userMeepleMove = (MeepleMove)m->data(0).value<void *>();
		userMoveReady = true;
	}
}

void BoardGraphicsScene::mouseMoveEvent(QGraphicsSceneMouseEvent * mouseEvent)
{
	int r = running;
	if (r == 1)
	{
		int x, y;
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

		if (item == 0)
		{
			placementTile->setPos(mouseEvent->scenePos());
		}
		else
		{
			placementTile->setPos(item->pos());
			int orientation = int(placementTile->rotation() / 90);
			QList<Board::TilePlacement> const & placements = *possiblePlacements;
#if defined(QT_NO_DEBUG) && !defined(QT_FORCE_ASSERTS)
			for (int i = 0; i < 4; ++i)
			{
#else
			for (int i = 0; i < 5; ++i)
			{
				Q_ASSERT(i < 4);
#endif
				for (Board::TilePlacement const & p : placements)
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

void BoardGraphicsScene::indexAt(const QPointF & scenePos, int & x, int & y)
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

void BoardGraphicsScene::boardChanged(const Board * board)
{
	qDeleteAll(openLayer->childItems());
	openTiles.clear();
	placementLayer->removeFromGroup(placementTile);
	qDeleteAll(meeplePlacementLayer->childItems());

	if (tileFactory == 0)
		return;

	uint arraySize = board->getInternalSize();
	setSceneRect(-TILE_SIZE / 2.0, -TILE_SIZE / 2.0, arraySize * TILE_SIZE, arraySize * TILE_SIZE);

	QList<QPoint> const & openPlaces = board->getOpenPlaces();

	for (uint y = 0; y < arraySize; ++y)
	{
		for (uint x = 0; x < arraySize; ++x)
		{
			Tile const * t = board->getTile(x, y);
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
	for (QPoint const & open : openPlaces)
	{
		uint x = open.x();
		uint y = open.y();

		QGraphicsRectItem * rect = new QGraphicsRectItem(-TILE_SIZE / 2.0, -TILE_SIZE / 2.0, TILE_SIZE, TILE_SIZE);
		rect->setPos(x * TILE_SIZE, y * TILE_SIZE);
		rect->setBrush(Qt::gray);
		rect->setData(0, x);
		rect->setData(1, y);

#if DRAW_TILE_POSITION_TEXT
		QGraphicsTextItem * text = new QGraphicsTextItem(QString("%1|%2").arg(x).arg(y), rect);
		text->setFont(QFont("sans", 90));
		text->setDefaultTextColor(Qt::red);
		text->setPos(-TILE_SIZE / 2.0, -90 / 2.0);
#endif

		openLayer->addToGroup(rect);
		openTiles.append(rect);
	}
}
