#include "game.h"

#include "player.h"
#include "jcz/tilefactory.h"

#include <QVarLengthArray>

Game::Game()
{
}

Game::~Game()
{
	cleanUp();
}

void Game::newGame(Tile::TileSets tileSets, jcz::TileFactory * tileFactory)
{
	if (players.size() == 0)
		return;

	cleanUp();

	ply = 0;
	tiles = tileFactory->createPack(tileSets, this);
	board = new Board(this, tiles.size());
	board->setStartTile(tiles.takeFirst());

	for (Player * p : allPlayers)
		p->newGame(this);

//	emit boardChanged(board);
}

void Game::addPlayer(Player * player)
{
	if (!isFinished())
		return;
	players.append(player);
	addWatchingPlayer(player);
}

void Game::addWatchingPlayer(Player * player)
{
	if (std::find(allPlayers.begin(), allPlayers.end(), player) == allPlayers.end()) // player not already in vector
		allPlayers.push_back(player);
}

//void Game::setPlayer(int index, Player * player)
//{
//	players[index] = player;
//}

void Game::setPlayers(QList<Player *> newPlayers)
{
	if (!isFinished())
		return;
	clearPlayers();
	players = newPlayers;
	for (Player * p : newPlayers)
		addWatchingPlayer(p);
}

void Game::clearPlayers()
{
	if (!isFinished())
		return;
	for (auto it = allPlayers.begin(); it < allPlayers.end(); )
	{
		if (players.contains(*it))
			it = allPlayers.erase(it);
		else
			++it;
	}
	players.clear();
}

Board const * Game::getBoard() const
{
	return board;
}

bool Game::isFinished()
{
	return ply < 0;
}

void Game::step()
{
	if (isFinished())
		return;

	qDebug() << "ply" << ply;

	Tile * tile = tiles.takeAt(r.nextInt(tiles.size()));
//	Tile * tile = tiles.takeAt(tiles.size() - 6);
	QList<TileMove> && placements = board->getPossibleTilePlacements(tile);
	while (placements.isEmpty())
	{
		delete tile;
		tile = tiles.takeAt(r.nextInt(tiles.size()));
		placements = board->getPossibleTilePlacements(tile);
	}
	qDebug() << "tile type:" << tile->tileType << "possible placements:" << placements.size();

	int playerIndex = currentPlayer = (currentPlayer + 1) % players.size();
	Player * player = players[playerIndex];

	TileMove move = player->getTileMove(tile, placements, this);
//	Move move{72, 73, Tile::left};
	qDebug() << "player" << playerIndex << move.x << move.y << move.orientation;
	tile->orientation = move.orientation;
	board->addTile(move.x, move.y, tile);

	QVarLengthArray<MeepleMove, NODE_ARRAY_LENGTH> possibleMeeples(1);
	possibleMeeples[0] = 0;
	for (auto n = tile->getCNodes(), end = n + tile->getNodeCount(); n < end; ++n)
		if (Util::isNodeFree(*n))
			possibleMeeples.append(*n);
	Q_ASSERT_X(possibleMeeples.size() <= NODE_ARRAY_LENGTH, "Game::step()", "possibleMeeples initial size too low");

	MeepleMove meepleMove = 0;
	if (possibleMeeples.size() > 1)
	{
		meepleMove = player->getMeepleMove(tile, possibleMeeples, this);
		if (meepleMove != 0)
		{
			Node * n = const_cast<Node *>(meepleMove);
			n->addMeeple(playerIndex);
			qDebug() << "player" << playerIndex << "placed meeple on" << n->t;
			tile->printSides(n);
		}
		else
			qDebug() << "player" << playerIndex << "placed no meeple";
	}

	for (Player * p : allPlayers)
		p->playerMoved(tile, move, meepleMove, this);

	if (tiles.isEmpty())
	{
		ply = -1;
		currentPlayer = -1;
	}
	else
		++ply;
}

void Game::cityClosed(CityNode * n)
{
	int score = n->tiles.size();
	if (score > 2)
		score = (score + n->bonus) * 2;
	qDebug() << "   city closed, value:" << score;
}

void Game::roadClosed(RoadNode * n)
{
	int score = n->tiles.size();
	qDebug() << "   raod closed, value:" << score;
}

void Game::cleanUp()
{
	delete board;
	qDeleteAll(tiles);
	tiles.clear();
}
