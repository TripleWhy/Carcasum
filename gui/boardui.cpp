#include "boardui.h"

BoardUI::BoardUI(JCZUtils::TileFactory * tileFactory, QWidget *parent) :
	QWidget(parent),
	game(0),
	running(false),
	userMoveReady(false),
	tileFactory(tileFactory)
{
}

BoardUI::~BoardUI()
{
}

QSize BoardUI::sizeHint() const
{
	return size;
}

void BoardUI::setGame(Game * g)
{
	if (game != 0)
	{
		disconnect(game, SIGNAL(boardChanged(const Board*)), this, SLOT(boardChanged(const Board*)));
	}

	game = g;

	connect(game, SIGNAL(boardChanged(const Board*)), this, SLOT(boardChanged(const Board*)));
}

void BoardUI::setTileFactory(JCZUtils::TileFactory * factory)
{
	tileFactory = factory;
}

Move BoardUI::getMove(const Tile * const tile, const QList<Board::TilePlacement> & placements, const Game * const /*game*/)
{
	if (running.exchange(true))
		return Move();

	foreach (TileUI * tui, openTiles) {
		tui->setOpenTile(tile);
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

	running = false;
	return Move(m.x, m.y, m.orientation);
}

void BoardUI::boardChanged(const Board * board)
{
	qDeleteAll(tiles);
	tiles.clear();
	openTiles.clear();

	if (tileFactory == 0)
		return;

	int arraySize = board->getInternalSize();
	int minX = arraySize;
	int maxX = 0;
	int minY = arraySize;
	int maxY = 0;

	for (int y = 0; y < arraySize; ++y)
	{
		for (int x = 0; x < arraySize; ++x)
		{
			if (board->getTile(x, y) != 0)
			{
				if (x < minX)
					minX = x;
				if (x > maxX)
					maxX = x;
				if (y < minY)
					minY = y;
				if (y > maxY)
					maxY = y;
			}
		}
	}

	QList<QPoint> const & openPlaces = board->getOpenPlaces();
	foreach (QPoint const & open, openPlaces)
	{
		if (open.x() < minX)
			minX = open.x();
		if (open.x() > maxX)
			maxX = open.x();
		if (open.y() < minY)
			minY = open.y();
		if (open.y() > maxY)
			maxY = open.y();
	}

	bool visible = isVisible();
	for (int y = 0; y <= (maxY - minY); ++y)
	{
		for (int x = 0; x <= (maxX - minX); ++x)
		{
			Tile const * t = board->getTile(x + minX, y + minY);
			if (t == 0)
				continue;

			TileUI * ui = new TileUI(t, tileFactory, this);
			tiles.append(ui);

			ui->setGeometry(x * TileUI::TILE_SIZE, y * TileUI::TILE_SIZE, TileUI::TILE_SIZE, TileUI::TILE_SIZE);
			if (visible)
				ui->setVisible(true);
		}
	}
	foreach (QPoint const & open, openPlaces)
	{
		int x = open.x();
		int y = open.y();
		TileUI * ui = new TileUI(x, y, tileFactory, this);
		ui->setText(QString("%1|%2").arg(x).arg(y));
		ui->setFont(QFont("sans", 13));
		tiles.append(ui);
		openTiles.append(ui);

		ui->setGeometry((x - minX) * TileUI::TILE_SIZE, (y - minY) * TileUI::TILE_SIZE, TileUI::TILE_SIZE, TileUI::TILE_SIZE);
		if (visible)
			ui->setVisible(true);

		connect(ui, SIGNAL(tilePlaced()), this, SLOT(tilePlaced()));
	}

	size.setWidth((maxX - minX + 1) * TileUI::TILE_SIZE);
	size.setHeight((maxX - minX + 1) * TileUI::TILE_SIZE);
	resize(size);
	setMinimumSize(size);
}

void BoardUI::tilePlaced()
{
	if (!running || userMoveReady)
		return;

	auto snd = static_cast<TileUI*>(sender());
	userMove = snd->getTilePlacement();
	userMoveReady = true;
}
