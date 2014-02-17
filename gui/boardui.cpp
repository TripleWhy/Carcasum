#include "boardui.h"

BoardUI::BoardUI(QWidget *parent) :
	QWidget(parent),
	game(0),
	size(0)
{
}

BoardUI::~BoardUI()
{
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

void BoardUI::boardChanged(const Board * board)
{
	if (size == 0)
	{
		TerrainType e[4];
		Tile t(Tile::BaseGame, 0, e);
		size = TileUI(&t).sizeHint().width();
	}

	qDeleteAll(tiles);
	tiles.clear();

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

	bool visible = isVisible();
	for (int y = 0; y <= (maxY - minY); ++y)
	{
		for (int x = 0; x <= (maxX - minX); ++x)
		{
			Tile const * t = board->getTile(x + minX, y + minY);
			if (t == 0)
				continue;

			TileUI * ui = new TileUI(t, this);
			tiles.append(ui);

			ui->setGeometry(x * size, y * size, size, size);
			if (visible)
				ui->setVisible(true);
		}
	}

	resize((maxX - minX + 1) * size, (maxX - minX + 1) * size);
}
