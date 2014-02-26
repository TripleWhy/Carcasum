#include "boardgraphicsscene.h"

#include <QGraphicsPixmapItem>

BoardGraphicsScene::BoardGraphicsScene(JCZUtils::TileFactory * tileFactory, QObject * parent)
	: QGraphicsScene(parent),
	  game(0),
	  tileFactory(tileFactory),
	  imgFactory(tileFactory),
	  running(false),
	  userMoveReady(false)
{
}

BoardGraphicsScene::~BoardGraphicsScene()
{
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

//	for (TileUI * tui : openTiles) {
//		tui->setOpenTile(tile);
//	}

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
	return Move(m.x, m.y, m.orientation);
}

void BoardGraphicsScene::boardChanged(const Board * board)
{
	qDeleteAll(tiles);
	tiles.clear();
//	openTiles.clear();

	if (tileFactory == 0)
		return;

	int arraySize = board->getInternalSize();
	setSceneRect(-TILE_SIZE / 2.0, -TILE_SIZE / 2.0, arraySize * TILE_SIZE, arraySize * TILE_SIZE);

	QList<QPoint> const & openPlaces = board->getOpenPlaces();

	for (int y = 0; y < arraySize; ++y)
	{
		for (int x = 0; x < arraySize; ++x)
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
//	for (QPoint const & open : openPlaces)
//	{
//		int x = open.x();
//		int y = open.y();
//		TileUI * ui = new TileUI(x, y, tileFactory, this);
//		ui->setText(QString("%1|%2").arg(x).arg(y));
//		ui->setFont(QFont("sans", 13));
//		tiles.append(ui);
//		openTiles.append(ui);

//		ui->setGeometry((x - minX) * TileUI::TILE_SIZE, (y - minY) * TileUI::TILE_SIZE, TileUI::TILE_SIZE, TileUI::TILE_SIZE);
//		if (visible)
//			ui->setVisible(true);

//		connect(ui, SIGNAL(tilePlaced()), this, SLOT(tilePlaced()));
//	}
}

void BoardGraphicsScene::tilePlaced()
{
	if (!running || userMoveReady)
		return;

//	auto snd = static_cast<TileUI*>(sender());
//	userMove = snd->getTilePlacement();
//	userMoveReady = true;
}
