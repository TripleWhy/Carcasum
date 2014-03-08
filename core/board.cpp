#include "board.h"
#include "game.h"

Board::Board(Game * game, const uint s)
	: game(game),
	  size(s * 2 - 1)//,
//	  offset(s - 1)
{
	board = new Tile ** [size];
	for (uint y = 0; y < size; ++y)
		board[y] = new Tile * [size]();

	//	board[offset][offset] = tiles.takeFirst();
}

Board::~Board()
{
	for (uint y = 0; y < size; ++y)
	{
		for (uint x = 0; x < size; ++x)
			delete board[x][y];
		delete[] board[y];
	}
	delete[] board;
}

Tile * Board::getTile(uint x, uint y)
{
//	return board[x + offset][y + offset];
	return board[x][y];
}

Tile const * Board::getTile(uint x, uint y) const
{
	return board[x][y];
}

void Board::setStartTile(Tile * tile)
{
	int offset = size / 2;
	board[offset][offset] = tile;

	open.insert(QPoint(offset - 1, offset), TerrainWrapper(None, None, tile->getEdge(Tile::left), None));
	open.insert(QPoint(offset + 1, offset), TerrainWrapper(tile->getEdge(Tile::right), None, None, None));
	open.insert(QPoint(offset, offset - 1), TerrainWrapper(None, None, None, tile->getEdge(Tile::up)));
	open.insert(QPoint(offset, offset + 1), TerrainWrapper(None, tile->getEdge(Tile::down), None, None));
//	open.insert(QPoint(-1,  0));
//	open.insert(QPoint(+1,  0));
//	open.insert(QPoint( 0, -1));
//	open.insert(QPoint( 0, +1));
}

void Board::addTile(uint x, uint y, Tile * tile)
{
	Q_ASSERT_X(x < size && y < size, "addTile", "position out of bounds");
	Q_ASSERT_X(board[x][y] == 0, "addTile", "position not empty");

	board[x][y] = tile;
	open.remove(QPoint(x, y));

	if (board[x - 1][y] == 0)
		open[QPoint(x - 1,  y)].t[Tile::right] = tile->getEdge(Tile::left);
	else
		board[x - 1][y]->connect(Tile::right, tile, game);

	if (board[x + 1][y] == 0)
		open[QPoint(x + 1,  y)].t[Tile::left] = tile->getEdge(Tile::right);
	else
		board[x + 1][y]->connect(Tile::left, tile, game);

	if (board[x][y - 1] == 0)
		open[QPoint(x,  y - 1)].t[Tile::down] = tile->getEdge(Tile::up);
	else
		board[x][y - 1]->connect(Tile::down, tile, game);

	if (board[x][y + 1] == 0)
		open[QPoint(x,  y + 1)].t[Tile::up] = tile->getEdge(Tile::down);
	else
		board[x][y + 1]->connect(Tile::up, tile, game);
	
	if (board[x - 1][y - 1])
		board[x - 1][y - 1]->connectDiagonal(tile, game);
	if (board[x + 1][y - 1])
		board[x + 1][y - 1]->connectDiagonal(tile, game);
	if (board[x - 1][y + 1])
		board[x - 1][y + 1]->connectDiagonal(tile, game);
	if (board[x + 1][y + 1])
		board[x + 1][y + 1]->connectDiagonal(tile, game);
}

uint Board::getInternalSize() const
{
	return size;
}

TileMovesType Board::getPossibleTilePlacements(const Tile * tile) const
{
	TileMovesType possible;
	TerrainType edges[4];
	quint32 rotations[4] = {0, 0, 0, 0};
	for (int orientation = 0; orientation < 4; ++orientation)
	{
		quint32 & r = rotations[orientation];
		for (int i = 0; i < 4; ++i)
		{
			TerrainType t = tile->getEdge((Tile::Side)i, (Tile::Side)orientation);
			edges[i] = t;
			r = (r << 8) | (quint32(t) & quint32(255));
		}
		for (int i = 0; i < orientation; ++i)
			if (r == rotations[i])
				goto hell2;

		for (QHash<QPoint, TerrainWrapper>::const_iterator it = open.constBegin(); it != open.constEnd(); ++it)
		{
			TerrainType const (&openTypes)[4] = it.value().t;
			for (int i = 0; i < 4; ++i)
			{
				if (openTypes[i] != None && openTypes[i] != edges[i])
					goto hell;
			}
			possible.append(TileMove{uint(it.key().x()), uint(it.key().y()), (Tile::Side)orientation});
			hell:;
		}
		hell2:;
	}

	return possible;
}

QList<QPoint> Board::getOpenPlaces() const
{
	return open.keys();
}

QPoint Board::positionOf(Tile * t) const
{
	for (uint y = 0; y < size; ++y)
		for (uint x = 0; x < size; ++x)
			if (board[x][y] == t)
				return QPoint(x, y);
	return QPoint(-1, -1);
}

void Board::scoreEndGame()
{
	std::unordered_set<Node *> scored;
	for (uint y = 0; y < size; ++y)
	{
		for (uint x = 0; x < size; ++x)
		{
			if (board[x][y])
			{
				for (Node * const * n = board[x][y]->getNodes(), * const * end = n + board[x][y]->getNodeCount(); n < end; ++n)
				{
					if ((*n)->isOccupied() && scored.find(*n) == scored.end())
					{
						scored.insert(*n);
						int const score = (*n)->getScore();
						if (score != 0)
							game->scoreNode(*n, score);
					}
				}
			}
		}
	}
}
