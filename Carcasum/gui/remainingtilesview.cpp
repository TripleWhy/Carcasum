#include "remainingtilesview.h"
#include "ui_remainingtilesview.h"

#include "core/util.h"
#include "core/game.h"
#include "remainingtileview.h"

RemainingTilesView::RemainingTilesView(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::RemainingTilesView)
{
	ui->setupUi(this);
}

RemainingTilesView::~RemainingTilesView()
{
	delete ui;
}

void RemainingTilesView::clear()
{
	for (QLayoutItem * item; (item = ui->remainingLayout->takeAt(0)) != 0; )
	{
		delete item->widget();
		delete item;
	}
	for (QLayoutItem * item; (item = ui->discardedLayout->takeAt(0)) != 0; )
	{
		delete item->widget();
		delete item;
	}
	views.clear();
	discardedViews.clear();
}

void RemainingTilesView::setUp(const Game * g, TileImageFactory * imgFactory)
{
	clear();
	game = g;
	this->imgFactory = imgFactory;

	Tile::TileSets const & sets = g->getTileSets();
	Tile::TileSets handled;
	std::map<Tile::TileSet, QGridLayout *> map;
	TileCountType const & tileCounts = g->getTileCounts();
	for (TileTypeType i = 0, s = (TileTypeType)tileCounts.size(); i < s; ++i)
	{
		Tile::TileSet set = Util::getSet(i);
		if (tileCounts[i] > 0 && sets.testFlag(set) && !handled.testFlag(set))
		{
			handled |= set;
			QLabel * lab = new QLabel(Util::getDisplayName(set));
			ui->remainingLayout->addWidget(lab);

			QGridLayout * layout = new QGridLayout();
			ui->remainingLayout->addLayout(layout);
			map[set] = layout;
		}
	}
	for (TileTypeType i = 0, s = (TileTypeType)tileCounts.size(); i < s; ++i)
	{
		views.append(0);
		Tile::TileSet set = Util::getSet(i);
		if (!handled.testFlag(set))
			continue;
		TileTypeType localType = Util::toLocalType(i);
		auto * rtv = new RemainingTileView(i, tileCounts[i], imgFactory);
		QGridLayout * layout = map[set];
		layout->addWidget(rtv, localType / REMAINING_TILES_COLUMNS, localType % REMAINING_TILES_COLUMNS);
		views[i] = rtv;
		rtv->setAttribute(Qt::WA_TransparentForMouseEvents);
	}
	ui->discardedWidget->setVisible(false);

	ui->tilesLeftLabel->setNum(g->getTileCount());
}

int RemainingTilesView::nextTile(const Game * game)
{
	if (running)
		return 0;
	running = true;
	ready = false;
	setMouseTracking(true);

	int t;
	forever
	{
		if (ready)
		{
			t = type;
			break;
		}
		else
		{
			if (Util::isGUIThread())
			{
				QCoreApplication::processEvents();
				Util::sleep(20);
			}
			else
				Util::sleep(200);
		}
	}
	setMouseTracking(false);

	int index = 0;
	auto const & counts = game->getTileCounts();
	for (int i = 0; i < counts.size(); ++i)
	{
		if (i >= t)
			break;
		index += counts[i];
	}
	Q_ASSERT(game->getTiles()[index]->tileType == t);

	running = false;
	return index;
}

void RemainingTilesView::leaveEvent(QEvent *)
{
	for (RemainingTileView * r : views)
		r->setHighlight(false);
}

void RemainingTilesView::mouseMoveEvent(QMouseEvent * event)
{
	if (!running || ready)
		return;

	RemainingTileView * rtv = getViewAt(event->pos());
	if (rtv == 0)
		return;

	for (RemainingTileView * r : views)
		r->setHighlight(false);

	if (rtv->getCount() <= 0)
		return;
	rtv->setHighlight(true);

	event->accept();
}

void RemainingTilesView::mousePressEvent(QMouseEvent * event)
{
	if (!running || ready)
	{
		QWidget::mousePressEvent(event);
		return;
	}

	RemainingTileView * rtv = getViewAt(event->pos());
	if (rtv == 0 || rtv->getCount() == 0)
	{
		QWidget::mousePressEvent(event);
		return;
	}

	for (RemainingTileView * r : views)
		r->setHighlight(false);

	event->accept();
	type = rtv->getType();
	ready = true;
}

void RemainingTilesView::updateView()
{
	TileCountType const & tileCounts = game->getTileCounts();
	for (int i = 0; i < tileCounts.size(); ++i)
	{
		if (views[i] != 0)
			views[i]->setCount(tileCounts[i]);
	}
	std::vector<Tile *> const & discardedTiles = game->getDiscardedTiles();
	if (discardedTiles.size() > 0)
	{
		ui->discardedWidget->setVisible(true);

		for (size_t i = discardedViews.size(), s = discardedTiles.size(); i < s; ++i)
		{
			Tile const * tile = discardedTiles[i];
			TileTypeType type = tile->tileType;
			auto * rtv = new RemainingTileView(type, -1, imgFactory);
			ui->discardedLayout->addWidget(rtv, (int)(i / REMAINING_TILES_COLUMNS), (int)(i % REMAINING_TILES_COLUMNS));

			rtv->setAttribute(Qt::WA_TransparentForMouseEvents);
			discardedViews.append(rtv);
		}
	}

	ui->tilesLeftLabel->setNum(game->getTileCount());
}

RemainingTileView * RemainingTilesView::getViewAt(const QPoint & pos)
{
//	QWidget * c = childAt(pos);
//	while(c != 0 && c->parent() != this)
//		c = c->parentWidget();
//	if (c == 0)
//		return 0;
//	return static_cast<RemainingTileView *>(c);

	// Oh man, sometimes I hate Qt... The solution above would work, if I hadn't set Qt::WA_TransparentForMouseEvents. Now the widgets become also invisible for mouse events, but also for childAt().
	// Other solutions using event filters etc. Also don't work, without digging deep in the chilrens widgets properties (e.g. to enable mouse tracking fall all children)

	for (RemainingTileView * rtv : views)
		if (rtv->geometry().contains(pos))
			return rtv;
	return 0;
}
