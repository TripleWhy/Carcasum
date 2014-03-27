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
	views.clear();
}

void RemainingTilesView::setUp(const Game * g, TileImageFactory * imgFactory)
{
	clear();
	game = g;

	Tile::TileSets const & sets = g->getTileSets();
	Tile::TileSets handled;
	std::map<Tile::TileSet, QGridLayout *> map;
	Game::TileCountType const & tileCounts = g->getTileCounts();
	for (int i = 0; i < tileCounts.size(); ++i)
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
	for (int i = 0; i < tileCounts.size(); ++i)
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
	}
	ui->discargedWidget->setVisible(false);
}

void RemainingTilesView::updateView()
{
	Game::TileCountType const & tileCounts = game->getTileCounts();
	for (int i = 0; i < tileCounts.size(); ++i)
	{
		if (views[i] != 0)
			views[i]->setCount(tileCounts[i]);
	}
	std::vector<Tile *> const & discargedTiles = game->getDiscargedTiles();
	if (discargedTiles.size() > 0)
	{
		//TODO
		ui->discargedWidget->setVisible(true);
	}
}
