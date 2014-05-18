#include "board.h"

Board::Board(Game * game, const uint s)
	: game(game),
	  size(s * 2 - 1)
{
	board = new Tile ** [size];
	for (uint y = 0; y < size; ++y)
		board[y] = new Tile * [size]();
}

Board::~Board()
{
	for (uint i = 0; i < size; ++i)
	{
		for (uint j = 0; j < size; ++j)
			delete board[i][j];
		delete[] board[i];
	}
	delete[] board;
}

void Board::setStartTile(Tile * tile)
{
	int offset = size / 2;
	board[offset][offset] = tile;
	tiles.push_back(tile);

	open.insert(QPoint(offset - 1, offset), EdgeMask(None, None, tile->getEdge(Tile::left), None));
	open.insert(QPoint(offset + 1, offset), EdgeMask(tile->getEdge(Tile::right), None, None, None));
	open.insert(QPoint(offset, offset - 1), EdgeMask(None, None, None, tile->getEdge(Tile::up)));
	open.insert(QPoint(offset, offset + 1), EdgeMask(None, tile->getEdge(Tile::down), None, None));
//	open.insert(QPoint(-1,  0));
//	open.insert(QPoint(+1,  0));
//	open.insert(QPoint( 0, -1));
//	open.insert(QPoint( 0, +1));
}

void Board::addTile(Tile * tile, TileMove const & move)
{
	uint const x = move.x;
	uint const y = move.y;
	Q_ASSERT_X(x < size && y < size, "addTile", "position out of bounds");
	Q_ASSERT_X(board[x][y] == 0, "addTile", "position not empty");

	tile->orientation = move.orientation;
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

	tiles.push_back(tile);
}

void Board::removeTile(const TileMove & move)
{
	uint const x = move.x;
	uint const y = move.y;
	Q_ASSERT_X(x < size && y < size, "removeTile", "position out of bounds");
	Q_ASSERT_X(board[x][y] != 0, "removeTile", "position empty");

	Tile * tile = board[x][y];

	auto it = std::find(tiles.crbegin(), tiles.crend(), tile);
	Q_ASSERT(it == tiles.crbegin());
	Q_ASSERT(it != tiles.crend());
	if (it != tiles.rend())
		tiles.erase( --(it.base()) );
	
	if (board[x + 1][y + 1])
		board[x + 1][y + 1]->disconnectDiagonal(tile, game);
	if (board[x - 1][y + 1])
		board[x - 1][y + 1]->disconnectDiagonal(tile, game);
	if (board[x + 1][y - 1])
		board[x + 1][y - 1]->disconnectDiagonal(tile, game);
	if (board[x - 1][y - 1])
		board[x - 1][y - 1]->disconnectDiagonal(tile, game);
	
	
	EdgeMask & tw = open[QPoint(x, y)];
	if (board[x][y + 1] == 0)
	{
		QPoint const & p = QPoint(x,  y + 1);
		EdgeMask & t = open[p];
		t.t[Tile::up] = None;
		if (t.t[0] == None && t.t[1] == None && t.t[2] == None && t.t[3] == None)
			open.remove(p);
	}
	else
	{
		Tile * t = board[x][y + 1];
		t->disconnect(Tile::up, tile, game);
		tw.t[Tile::down] = t->getEdge(Tile::up);
	}
	
	if (board[x][y - 1] == 0)
	{
		QPoint const & p = QPoint(x,  y - 1);
		EdgeMask & t = open[p];
		t.t[Tile::down] = None;
		if (t.t[0] == None && t.t[1] == None && t.t[2] == None && t.t[3] == None)
			open.remove(p);
	}
	else
	{
		Tile * t = board[x][y - 1];
		t->disconnect(Tile::down, tile, game);
		tw.t[Tile::up] = t->getEdge(Tile::down);
	}
	
	if (board[x + 1][y] == 0)
	{
		QPoint const & p = QPoint(x + 1,  y);
		EdgeMask & t = open[p];
		t.t[Tile::left] = None;
		if (t.t[0] == None && t.t[1] == None && t.t[2] == None && t.t[3] == None)
			open.remove(p);
	}
	else
	{
		Tile * t = board[x + 1][y];
		t->disconnect(Tile::left, tile, game);
		tw.t[Tile::right] = t->getEdge(Tile::left);
	}
	
	if (board[x - 1][y] == 0)
	{
		QPoint const & p = QPoint(x - 1,  y);
		EdgeMask & t = open[p];
		t.t[Tile::right] = None;
		if (t.t[0] == None && t.t[1] == None && t.t[2] == None && t.t[3] == None)
			open.remove(p);
	}
	else
	{
		Tile * t = board[x - 1][y];
		t->disconnect(Tile::right, tile, game);
		tw.t[Tile::left] = t->getEdge(Tile::right);
	}
	
	board[x][y] = 0;
	tile->orientation = Tile::Side();	//TODO this is probably not neccessary
}

uint Board::getInternalSize() const
{
	return size;
}

void Board::clear()
{
	for (uint i = 0; i < size; ++i)
	{
		for (uint j = 0; j < size; ++j)
		{
			delete board[i][j];
			board[i][j] = 0;
		}
	}
	open.clear();
	tiles.clear();
}

void Board::reset()
{
	for (uint i = 0; i < size; ++i)
	{
		for (uint j = 0; j < size; ++j)
		{
			board[i][j] = 0;
		}
	}
	open.clear();
	tiles.clear();
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

		for (QHash<QPoint, EdgeMask>::const_iterator it = open.constBegin(); it != open.constEnd(); ++it)
		{
			TerrainType const (&openTypes)[4] = it.value().t;
			for (int i = 0; i < 4; ++i)
			{
				if (openTypes[i] != None && openTypes[i] != edges[i])
					goto hell;
			}
			possible.push_back(TileMove{uint(it.key().x()), uint(it.key().y()), (Tile::Side)orientation});
			hell:;
		}
		hell2:;
	}
	Q_ASSERT_X(possible.size() <= TILE_ARRAY_LENGTH, "Board::getPossibleTilePlacements()", "placements initial size too low");

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
	for (Tile * tile : tiles)
		for (Node * const * n = tile->getNodes(), * const * end = n + tile->getNodeCount(); n < end; ++n)
		{
			if ((*n)->isOccupied())
			{
				game->scoreNodeEndGame(*n);
			}
		}
}

void Board::unscoreEndGame()
{
	for (Tile * tile : tiles)
		for (Node * const * n = tile->getNodes(), * const * end = n + tile->getNodeCount(); n < end; ++n)
		{
			if ((*n)->isOccupied())
			{
				game->unscoreNodeEndGame(*n);
			}
		}
}

std::vector<int> Board::countUnscoredMeeples() const
{
	std::vector<int> meeples;
	for (uint i = 0; i < game->getPlayerCount(); ++i)
		meeples.push_back(0);
	std::unordered_set<Node::NodeData const *> nodeIds;
	for (Tile * tile : tiles)
	{
		for (Node * const * n = tile->getNodes(), * const * end = n + tile->getNodeCount(); n < end; ++n)
		{
			if ((*n)->getScored() == NotScored && nodeIds.find((*n)->getData()) == nodeIds.end())
			{
				nodeIds.insert((*n)->getData());

				QString s;
				for (uint i = 0; i < game->getPlayerCount(); ++i)
				{
					meeples[i] += (*n)->getMeeples()[i];
					s = s + ", " + QString::number((*n)->getMeeples()[i]);
				}
//				qDebug() << "unscored" << (*n)->getTerrain() << "\t" << (*n)->id() << "on tile" << tile->id << "\t" << s;
			}
		}
	}
	return meeples;
}

bool Board::equals(const Board & other) const
{
	QMap<Tile *, Tile *> tileMap;
	if (size != other.size)
		return false;
	for (uint y = 0; y < size; ++y)
	{
		for (uint x = 0; x < size; ++x)
		{
			if ((board[x][y] == 0) != (other.board[x][y] == 0))
				return false;
			if (board[x][y] != 0)
			{
				if (!board[x][y]->equals(*other.board[x][y], game))
					return false;
				tileMap[board[x][y]] = other.board[x][y];
			}
		}
	}
	if (open != other.open)
		return false;
	if (tiles.size() != other.tiles.size())
		return false;
	for (uint i = 0; i < tiles.size(); ++i)
		if (tileMap[tiles[i]] != other.tiles[i])
			return false;
	return true;
}
