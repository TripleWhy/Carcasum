#include "boardgraphicsscene.h"

#include <QGraphicsPixmapItem>
#include <QGraphicsSceneMouseEvent>

BoardGraphicsScene::BoardGraphicsScene(JCZUtils::TileFactory * tileFactory, QObject * parent)
	: QGraphicsScene(parent),
	  game(0),
	  tileFactory(tileFactory),
	  imgFactory(tileFactory),
	  running(false),
	  userMoveReady(false)
{
	setBackgroundBrush(QBrush(QPixmap(":/jcz/sysimages/panel_bg.png")));

	placementTile = new QGraphicsPixmapItem();
	placementTile->setTransformationMode(Qt::SmoothTransformation);
	placementTile->setShapeMode(QGraphicsPixmapItem::BoundingRectShape);
	placementTile->setOffset(-TILE_SIZE / 2.0, -TILE_SIZE / 2.0);
	placementTile->setOpacity(0.7);
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

void BoardGraphicsScene::setTileFactory(JCZUtils::TileFactory * factory)
{
	tileFactory = factory;
}

Move BoardGraphicsScene::getMove(const Tile * const tile, const QList<Board::TilePlacement> & placements, const Game * const /*game*/)
{
	if (running.exchange(true))
		return Move();

	possiblePlacements = &placements;
	placementTile->setPixmap(imgFactory.getImage(tile));
	addItem(placementTile);
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
	placementTile->setVisible(true);

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

	running = false;
	removeItem(placementTile);
	return Move(m.x, m.y, m.orientation);
}

void BoardGraphicsScene::mousePressEvent(QGraphicsSceneMouseEvent * mouseEvent)
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

void BoardGraphicsScene::mouseMoveEvent(QGraphicsSceneMouseEvent * mouseEvent)
{
	if (running)
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
}

void BoardGraphicsScene::indexAt(QPointF scenePos, int & x, int & y)
{
	x = (scenePos.x() + TILE_SIZE / 2) / TILE_SIZE;
	y = (scenePos.y() + TILE_SIZE / 2) / TILE_SIZE;
}

void BoardGraphicsScene::boardChanged(const Board * board)
{
	qDeleteAll(tiles);
	tiles.clear();
	openTiles.clear();

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

			addItem(item);
			tiles.append(item);
		}
	}
	for (QPoint const & open : openPlaces)
	{
		uint x = open.x();
		uint y = open.y();

		QGraphicsRectItem * rect = new QGraphicsRectItem(-TILE_SIZE / 2.0, -TILE_SIZE / 2.0, TILE_SIZE, TILE_SIZE);
		rect->setPos(x * TILE_SIZE, y * TILE_SIZE);
		rect->setOpacity(0.2);
		rect->setBrush(Qt::gray);
		rect->setData(0, x);
		rect->setData(1, y);

#if DRAW_TILE_POSITION_TEXT
		QGraphicsTextItem * text = new QGraphicsTextItem(QString("%1|%2").arg(x).arg(y), rect);
		text->setFont(QFont("sans", 90));
		text->setDefaultTextColor(Qt::red);
		text->setPos(-TILE_SIZE / 2.0, -90 / 2.0);
#endif

		addItem(rect);
		tiles.append(rect);
		openTiles.insert(rect);
	}
}
