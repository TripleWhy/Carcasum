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

	open.insert(QPoint(offset - 1, offset), TerrainWrapper(None, None, tile->getEdge(Tile::left), None));
	open.insert(QPoint(offset + 1, offset), TerrainWrapper(tile->getEdge(Tile::right), None, None, None));
	open.insert(QPoint(offset, offset - 1), TerrainWrapper(None, None, None, tile->getEdge(Tile::up)));
	open.insert(QPoint(offset, offset + 1), TerrainWrapper(None, tile->getEdge(Tile::down), None, None));
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
}

void Board::removeTile(const TileMove & move)
{
	uint const x = move.x;
	uint const y = move.y;
	Q_ASSERT_X(x < size && y < size, "removeTile", "position out of bounds");
	Q_ASSERT_X(board[x][y] != 0, "removeTile", "position empty");

	Tile * tile = board[x][y];
	
	if (board[x + 1][y + 1])
		board[x + 1][y + 1]->disconnectDiagonal(tile, game);
	if (board[x - 1][y + 1])
		board[x - 1][y + 1]->disconnectDiagonal(tile, game);
	if (board[x + 1][y - 1])
		board[x + 1][y - 1]->disconnectDiagonal(tile, game);
	if (board[x - 1][y - 1])
		board[x - 1][y - 1]->disconnectDiagonal(tile, game);
	
	
	TerrainWrapper & tw = open[QPoint(x, y)];
	if (board[x][y + 1] == 0)
	{
		QPoint const & p = QPoint(x,  y + 1);
		TerrainWrapper & t = open[p];
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
		TerrainWrapper & t = open[p];
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
		TerrainWrapper & t = open[p];
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
		TerrainWrapper & t = open[p];
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
//	qDebug() << "SCORE END" << this << game;
	for (uint y = 0; y < size; ++y)
	{
		for (uint x = 0; x < size; ++x)
		{
			if (board[x][y])
			{
				for (Node * const * n = board[x][y]->getNodes(), * const * end = n + board[x][y]->getNodeCount(); n < end; ++n)
				{
					if ((*n)->isOccupied())
					{
						game->scoreNodeEndGame(*n);
					}
				}
			}
		}
	}
}

void Board::unscoreEndGame()
{
//	qDebug() << "UNSCORE END" << this << game;
	for (uint y = 0; y < size; ++y)
	{
		for (uint x = 0; x < size; ++x)
		{
			if (board[x][y])
			{
				for (Node * const * n = board[x][y]->getNodes(), * const * end = n + board[x][y]->getNodeCount(); n < end; ++n)
				{
					if ((*n)->isOccupied())
					{
						game->unscoreNodeEndGame(*n);
					}
				}
			}
		}
	}
}

std::vector<int> Board::countUnscoredMeeples() const
{
	std::vector<int> meeples;
	for (uint i = 0; i < game->getPlayerCount(); ++i)
		meeples.push_back(0);
	std::unordered_set<Node::NodeData const *> nodeIds;
	for (uint y = 0; y < size; ++y)
	{
		for (uint x = 0; x < size; ++x)
		{
			if (board[x][y] != 0)
			{
				for (Node * const * n = board[x][y]->getNodes(), * const * end = n + board[x][y]->getNodeCount(); n < end; ++n)
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
//						qDebug() << "unscored" << (*n)->getTerrain() << "\t" << (*n)->id() << "on tile" << board[x][y]->id << "\t" << s;
					}
				}
			}
		}
	}
	return meeples;
}

bool Board::equals(const Board & other) const
{
	if (size != other.size)
		return false;
	for (uint y = 0; y < size; ++y)
	{
		for (uint x = 0; x < size; ++x)
		{
			if ((board[x][y] == 0) != (other.board[x][y] == 0))
				return false;
			if (board[x][y] != 0 && !board[x][y]->equals(*other.board[x][y], game))
				return false;
		}
	}
	if (open != other.open)
		return false;
	return true;
}
