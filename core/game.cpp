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
	int const playerCount = getPlayerCount();
	if (playerCount == 0)
		return;

	cleanUp();

	playerScores = new int[playerCount]();
	returnMeeples = new int[playerCount]();
	playerMeeples = new int[playerCount];
	for (int i = 0; i < playerCount; ++i)
		playerMeeples[i] = 7;
	
	ply = 0;
	tiles = tileFactory->createPack(tileSets, this);
	board = new Board(this, tiles.size());
	board->setStartTile(tiles.takeFirst());

	for (uint i = 0; i < allPlayers.size(); ++i)
		allPlayers[i]->newGame(i, this);
}

void Game::addPlayer(Player * player)
{
	if (!isFinished())
		return;
	players.append(player);
	addWatchingPlayer(player);
	playerCount = players.size();
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
	playerCount = players.size();
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
	playerCount = 0;
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

	int const playerIndex = currentPlayer = (currentPlayer + 1) % getPlayerCount();
	Player * player = players[playerIndex];

	TileMove move = player->getTileMove(playerIndex, tile, placements, this);
//	Move move{72, 73, Tile::left};
	qDebug() << "player" << playerIndex << move.x << move.y << move.orientation;
	tile->orientation = move.orientation;
	board->addTile(move.x, move.y, tile);

	MeepleMove meepleMove = 0;
	if (playerMeeples[playerIndex] > 0)
	{
		QVarLengthArray<MeepleMove, NODE_ARRAY_LENGTH> possibleMeeples(1);
		possibleMeeples[0] = 0;
		for (auto n = tile->getCNodes(), end = n + tile->getNodeCount(); n < end; ++n)
			if (Util::isNodeFree(*n))
				possibleMeeples.append(*n);
		Q_ASSERT_X(possibleMeeples.size() <= NODE_ARRAY_LENGTH, "Game::step()", "possibleMeeples initial size too low");
	
		if (possibleMeeples.size() > 1)
		{
			meepleMove = player->getMeepleMove(playerIndex, tile, possibleMeeples, this);
			if (meepleMove != 0)
			{
				Node * n = const_cast<Node *>(meepleMove);
				n->addMeeple(playerIndex, this);
				--playerMeeples[playerIndex];
				
				qDebug() << "player" << playerIndex << "placed meeple on" << n->t;
				tile->printSides(n);
			}
			else
				qDebug() << "player" << playerIndex << "placed no meeple";
		}
	}
	for (int * r = returnMeeples, * end = r + getPlayerCount(), * m = playerMeeples; r < end; ++r, ++m)
	{
		*m += *r;
		*r = 0;
	}

	for (Player * p : allPlayers)
		p->playerMoved(playerIndex, tile, move, meepleMove, this);

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
	
	scoreNode(n, score);
}

void Game::roadClosed(RoadNode * n)
{
	int score = n->tiles.size();
	qDebug() << "   raod closed, value:" << score;
	
	scoreNode(n, score);
}

void Game::scoreNode(Node * n, int const score)
{
	uchar meepleCount = n->getMaxMeeples();
	for (uchar * m = n->meeples, * end = m + getPlayerCount(), player = 0; m < end; ++m, ++player)
	{
		if (*m == meepleCount)
			playerScores[player] += score;
		returnMeeples[player] += *m;
		*m = 0;
	}
	
	qDebug() << "Player scores:";
	for (int i = 0; i < getPlayerCount(); ++i)
		qDebug() << "  Player" << i << ":" << playerScores[i];
}

void Game::cleanUp()
{
	delete board;
	qDeleteAll(tiles);
	tiles.clear();
	delete[] playerScores;
	delete[] playerMeeples;
	delete[] returnMeeples;
}
