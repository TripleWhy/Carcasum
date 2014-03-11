#include "game.h"
#include "board.h"
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
	this->tileSets = tileSets;
	tiles = tileFactory->createPack(tileSets, this);
	board = new Board(this, tiles.size());
	board->setStartTile(tiles.takeFirst());

	for (uint i = 0; i < allPlayers.size(); ++i)
		allPlayers[i]->newGame(i, this);
}

void Game::newGame(Tile::TileSets tileSets, jcz::TileFactory * tileFactory, const std::vector<MoveHistoryEntry> & history)
{
	newGame(tileSets, tileFactory);
	applyHistory(history);
}

void Game::restartGame(jcz::TileFactory * tileFactory)
{
	moveHistory.clear();
	qDeleteAll(tiles);
	tiles.clear();
	
	int const playerCount = getPlayerCount();
	memset(playerScores, 0, sizeof(*playerScores) * playerCount);
	memset(returnMeeples, 0, sizeof(*returnMeeples) * playerCount);
	for (int i = 0; i < playerCount; ++i)
		playerMeeples[i] = 7;
	
	ply = 0;
	tiles = tileFactory->createPack(tileSets, this);
	board->clear();
	board->setStartTile(tiles.takeFirst());

//	for (uint i = 0; i < allPlayers.size(); ++i)
	//		allPlayers[i]->newGame(i, this);
}

void Game::restartGame(jcz::TileFactory * tileFactory, const std::vector<MoveHistoryEntry> & history)
{
	restartGame(tileFactory);
	applyHistory(history);
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

//	qDebug() << "ply" << ply;

	MoveHistoryEntry entry;
	entry.tile = r.nextInt(tiles.size());
	Tile * tile = tiles.takeAt(entry.tile);
	TileMovesType && placements = board->getPossibleTilePlacements(tile);
	while (placements.isEmpty())
	{
		delete tile;
		
		moveHistory.push_back(entry);
		entry.tile = r.nextInt(tiles.size());
		tile = tiles.takeAt(entry.tile);
		placements = board->getPossibleTilePlacements(tile);
	}
	Q_ASSERT_X(placements.size() <= TILE_ARRAY_LENGTH, "Game::step()", "placements initial size too low");
//	qDebug() << "tile type:" << tile->tileType << "possible placements:" << placements.size();

	int const playerIndex = currentPlayer = (currentPlayer + 1) % getPlayerCount();
	Player * player = players[playerIndex];

	Move & move = entry.move;
	TileMove & tileMove = move.tileMove;
	tileMove = player->getTileMove(playerIndex, tile, entry, placements, this);
//	qDebug() << "player" << playerIndex << tileMove.x << tileMove.y << tileMove.orientation;
	board->addTile(tile, tileMove);

	MeepleMove & meepleMove = move.meepleMove;
	if (playerMeeples[playerIndex] > 0)
	{
		MeepleMovesType possibleMeeples(1);
//		possibleMeeples[0] = MeepleMove();	// Not neccessary for non-primitive types
		
		auto nodes = tile->getNodes();
		for (uchar i = 0, end = tile->getNodeCount(); i < end; ++i)
			if (Util::isNodeFree(nodes[i]))
				possibleMeeples.append(MeepleMove(i));
		Q_ASSERT_X(possibleMeeples.size() <= NODE_ARRAY_LENGTH, "Game::step()", "possibleMeeples initial size too low");
	
		if (possibleMeeples.size() > 1)
		{
			meepleMove = player->getMeepleMove(playerIndex, tile, entry, possibleMeeples, this);
			if (!meepleMove.isNull())
			{
				Node * n = nodes[meepleMove.nodeIndex];
				n->addMeeple(playerIndex, this);
				--playerMeeples[playerIndex];
				
//				qDebug() << "player" << playerIndex << "placed meeple on" << n->t;
//				tile->printSides(n);
			}
//			else
//				qDebug() << "player" << playerIndex << "placed no meeple";
		}
	}
	for (int * r = returnMeeples, * end = r + getPlayerCount(), * m = playerMeeples; r < end; ++r, ++m)
	{
		*m += *r;
		*r = 0;
	}
	
	moveHistory.push_back(entry);
	for (Player * p : allPlayers)
		p->playerMoved(playerIndex, tile, entry, this);

	if (tiles.isEmpty())
	{
		board->scoreEndGame();
		
		ply = -1;
		currentPlayer = -1;
	}
	else
		++ply;
}

void Game::step(int tileIndex, const TileMove & tileMove, int playerIndex, Player * player)
{
	MoveHistoryEntry entry;
	entry.tile = tileIndex;
	Tile * tile = tiles.takeAt(entry.tile);

	Move & move = entry.move;
	move.tileMove = tileMove;
	board->addTile(tile, tileMove);

	MeepleMove & meepleMove = move.meepleMove;
	if (playerMeeples[playerIndex] > 0)
	{
		MeepleMovesType possibleMeeples(1);
		
		auto nodes = tile->getNodes();
		for (uchar i = 0, end = tile->getNodeCount(); i < end; ++i)
			if (Util::isNodeFree(nodes[i]))
				possibleMeeples.append(MeepleMove(i));
		Q_ASSERT_X(possibleMeeples.size() <= NODE_ARRAY_LENGTH, "Game::step()", "possibleMeeples initial size too low");
	
		if (possibleMeeples.size() > 1)
		{
			meepleMove = player->getMeepleMove(playerIndex, tile, entry, possibleMeeples, this);
			if (!meepleMove.isNull())
			{
				Node * n = nodes[meepleMove.nodeIndex];
				n->addMeeple(playerIndex, this);
				--playerMeeples[playerIndex];
			}
		}
	}
	for (int * r = returnMeeples, * end = r + getPlayerCount(), * m = playerMeeples; r < end; ++r, ++m)
	{
		*m += *r;
		*r = 0;
	}
	
	moveHistory.push_back(entry);
//	for (Player * p : allPlayers)
//		p->playerMoved(playerIndex, tile, entry, this);

	if (tiles.isEmpty())
	{
		board->scoreEndGame();
		
		ply = -1;
		currentPlayer = -1;
	}
	else
		++ply;
}

void Game::step(const MoveHistoryEntry & entry)
{
//	if (isFinished())
//		return;
	
	Tile * tile = tiles.takeAt(entry.tile);
	if (entry.move.tileMove.isNull())
	{
		delete tile;
		return;
	}

	int const playerIndex = currentPlayer = (currentPlayer + 1) % getPlayerCount();

	Move const & move = entry.move;
	TileMove const & tileMove = move.tileMove;
	board->addTile(tile, tileMove);

	auto nodes = tile->getNodes();
	MeepleMove const & meepleMove = move.meepleMove;
	if (!meepleMove.isNull())
	{
		Node * n = nodes[meepleMove.nodeIndex];
		n->addMeeple(playerIndex, this);
		--playerMeeples[playerIndex];
	}
//	for (int * r = returnMeeples, * end = r + getPlayerCount(), * m = playerMeeples; r < end; ++r, ++m)
//	{
//		*m += *r;
//		*r = 0;
//	}

	moveHistory.push_back(entry);
//	for (Player * p : allPlayers)
//		p->playerMoved(playerIndex, tile, move, this);

//	if (tiles.isEmpty())
//	{
//		board->scoreEndGame();
		
//		ply = -1;
//		currentPlayer = -1;
//	}
//	else
		++ply;
}

void Game::cityClosed(CityNode * n)
{
	int score = n->getScore();
	if (score > 2)
		score = (score + n->bonus) * 2;
//	qDebug() << "   city closed, value:" << score;
	
	scoreNode(n, score);
}

void Game::roadClosed(RoadNode * n)
{
	int score = n->getScore();
//	qDebug() << "   raod closed, value:" << score;
	
	scoreNode(n, score);
}

void Game::cloisterClosed(CloisterNode * n)
{
	static int const score = 9; //n->getScore()
//	qDebug() << "   cloister closed, value:" << score;
	
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
	
//	qDebug() << "Player scores:";
//	for (int i = 0; i < getPlayerCount(); ++i)
//		qDebug() << "  Player" << i << ":" << playerScores[i];
}

void Game::cleanUp()
{
	moveHistory.clear();
	
	delete board;
	qDeleteAll(tiles);
	tiles.clear();
	delete[] playerScores;
	delete[] playerMeeples;
	delete[] returnMeeples;
}

void Game::applyHistory(const std::vector<MoveHistoryEntry> & history)
{
	for (MoveHistoryEntry const & e : history)
		step(e);
	for (int * r = returnMeeples, * end = r + getPlayerCount(), * m = playerMeeples; r < end; ++r, ++m)
	{
		*m += *r;
		*r = 0;
	}
}
