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
	
	active = true;
	nextPlayer = 0;
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
	qDeleteAll(discardedTiles);
	discardedTiles.clear();
	
	int const playerCount = getPlayerCount();
	memset(playerScores, 0, sizeof(*playerScores) * playerCount);
	memset(returnMeeples, 0, sizeof(*returnMeeples) * playerCount);
	for (int i = 0; i < playerCount; ++i)
		playerMeeples[i] = 7;
	
	active = true;
	nextPlayer = 0;
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

void Game::step()
{
	if (isFinished())
		return;

//	qDebug() << "ply" << ply;
	qDebug() << "STEP:";

	MoveHistoryEntry entry;
	entry.tile = r.nextInt(tiles.size());
	Tile * tile = tiles[entry.tile];
	TileMovesType && placements = board->getPossibleTilePlacements(tile);
	while (placements.isEmpty())
	{
		qDebug() << moveHistory.size() << "discarged Tile" << tile->tileType << "total:" << discardedTiles.size();
		discardedTiles.push_back(tile);
		moveHistory.push_back(entry);
		tiles.removeAt(entry.tile);
		
		entry.tile = r.nextInt(tiles.size());
		tile = tiles.takeAt(entry.tile);
		placements = board->getPossibleTilePlacements(tile);
	}
	Q_ASSERT_X(placements.size() <= TILE_ARRAY_LENGTH, "Game::step()", "placements initial size too low");
//	qDebug() << "tile type:" << tile->tileType << "possible placements:" << placements.size();

	int const playerIndex = nextPlayer;
	Player * player = players[playerIndex];

	Move & move = entry.move;
	TileMove & tileMove = move.tileMove;
	tileMove = player->getTileMove(playerIndex, tile, entry, placements, this);
//	qDebug() << moveHistory.size() << "player" << playerIndex << tileMove.x << tileMove.y << tileMove.orientation;
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
				
//				qDebug() << moveHistory.size() << "player" << playerIndex << "placed meeple on" << n->t;
//				tile->printSides(n);
			}
//			else
//				qDebug() << moveHistory.size() << "player" << playerIndex << "placed no meeple";
		}
	}
	for (int * r = returnMeeples, * end = r + getPlayerCount(), * m = playerMeeples; r < end; ++r, ++m)
	{
		*m += *r;
		*r = 0;
	}
	
	nextPlayer = (nextPlayer + 1) % getPlayerCount();
	
	for (int i = 0; i < getPlayerCount(); ++i)
		qDebug() << moveHistory.size() << "player" << i << "score:" << playerScores[i] << "meeples:" << playerMeeples[i];
	qDebug() << "next player:" << nextPlayer;
	qDebug() << moveHistory.size() << "player" << playerIndex << tileMove.x << tileMove.y << tileMove.orientation;
	if (!meepleMove.isNull())
		qDebug() << (moveHistory.size()) << "player" << playerIndex << "placed meeple on" << tile->getNodes()[meepleMove.nodeIndex]->t;
	else
		qDebug() << (moveHistory.size()) << "player" << playerIndex << "placed no meeple";
	qDebug();
	
	moveHistory.push_back(entry);
	for (Player * p : allPlayers)
		p->playerMoved(playerIndex, tile, entry, this);

	tiles.removeAt(entry.tile);
	if (tiles.isEmpty())
	{
		board->scoreEndGame();
		active = false;
		
		for (int * r = returnMeeples, * end = r + getPlayerCount(), * m = playerMeeples; r < end; ++r, ++m)
		{
			*m += *r;
			*r = 0;
		}
		
		qDebug("GAME OVER:");
		for (int i = 0; i < getPlayerCount(); ++i)
			qDebug() << moveHistory.size() << "player" << i << "score:" << playerScores[i] << "meeples:" << playerMeeples[i];
	}
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
		
		active = false;
	}
}

void Game::step(const MoveHistoryEntry & entry)
{
//	if (isFinished())
//		return;
	
	Tile * tile = tiles.takeAt(entry.tile);
	if (entry.move.tileMove.isNull())
	{
		discardedTiles.push_back(tile);
		return;
	}

	int const playerIndex = nextPlayer;

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

	if (tiles.isEmpty())
	{
		board->scoreEndGame();
		active = false;
	}
	
	nextPlayer = (nextPlayer + 1) % getPlayerCount();
}

void Game::undo()
{
	Q_ASSERT(moveHistory.size() > 0);
	
	MoveHistoryEntry const & entry = moveHistory.back();
	moveHistory.pop_back();

	if (tiles.isEmpty())
	{
		board->unscoreEndGame();
		active = true;
	}
	
	Move const & move = entry.move;
	TileMove const & tileMove = move.tileMove;
	
	if (tileMove.isNull())
	{
		Tile * t = discardedTiles.back();
		discardedTiles.pop_back();
		tiles.insert(entry.tile, t);
		
		qDebug() << (moveHistory.size()-1) << "discarged Tile" << t->tileType << "total:" << discardedTiles.size();
		return;
	}
	
	Tile * tile = board->getTile(tileMove);
	
	nextPlayer = (nextPlayer - 1 + getPlayerCount()) % getPlayerCount();
	int const playerIndex = nextPlayer;
	MeepleMove const & meepleMove = move.meepleMove;
	if (!meepleMove.isNull())
	{
		auto nodes = tile->getNodes();
		Node * n = nodes[meepleMove.nodeIndex];
		++playerMeeples[playerIndex];
		Q_ASSERT(playerMeeples[playerIndex] <= 7);
		n->removeMeeple(playerIndex, this);
	}
	
	board->removeTile(tileMove);
	
	tiles.insert(entry.tile, tile);
	
//	for (int * r = returnMeeples, * end = r + getPlayerCount(), * m = playerMeeples; r < end; ++r, ++m)
//	{
//		*m += *r;
//		*r = 0;
//	}
	
	qDebug() << (moveHistory.size()) << "player" << playerIndex << tileMove.x << tileMove.y << tileMove.orientation;
	if (!meepleMove.isNull())
		qDebug() << (moveHistory.size()) << "player" << playerIndex << "placed meeple on" << tile->getNodes()[meepleMove.nodeIndex]->t;
	else
		qDebug() << (moveHistory.size()) << "player" << playerIndex << "placed no meeple";
	qDebug();
	
	qDebug() << "UNDO:";
	for (int i = 0; i < getPlayerCount(); ++i)
		qDebug() << (moveHistory.size()-1) << "player" << i << "score:" << playerScores[i] << "meeples:" << playerMeeples[i];
	qDebug() << "next player:" << nextPlayer;
}

void Game::cityClosed(CityNode * n)
{
	int score = n->getScore();
	if (score > 2)
		score *= 2;
//	qDebug() << "   city closed, value:" << score;
	
	scoreNode(n, score);
}

void Game::cityUnclosed(CityNode * n)
{
	int score = n->getScore();
	if (score > 2)
		score *= 2;
	
	unscoreNode(n, score);
}

void Game::roadClosed(RoadNode * n)
{
	int score = n->getScore();
//	qDebug() << "   raod closed, value:" << score;
	scoreNode(n, score);
}

void Game::roadUnclosed(RoadNode * n)
{
	int score = n->getScore();
	unscoreNode(n, score);
}

void Game::cloisterClosed(CloisterNode * n)
{
	static int const score = 9; //n->getScore()
//	qDebug() << "   cloister closed, value:" << score;
	
	scoreNode(n, score);
}

void Game::cloisterUnclosed(CloisterNode * n)
{
	static int const score = 9;
	unscoreNode(n, score);
}

void Game::scoreNode(Node * n, int const score)
{
	uchar meepleCount = n->getMaxMeeples();
	int player = 0;
	for (uchar const * m = n->getMeeples(), * end = m + getPlayerCount(); m < end; ++m, ++player)
	{
		if (*m == meepleCount)
			playerScores[player] += score;
		returnMeeples[player] += *m;
	}
	
//	qDebug() << "Player scores:";
//	for (int i = 0; i < getPlayerCount(); ++i)
	//		qDebug() << "  Player" << i << ":" << playerScores[i];
}

void Game::unscoreNode(Node * n, const int score)
{
	uchar meepleCount = n->getMaxMeeples();
	int player = 0;
	for (uchar const * m = n->getMeeples(), * end = m + getPlayerCount(); m < end; ++m, ++player)
	{
		if (*m == meepleCount)
		{
			playerScores[player] -= score;
			Q_ASSERT(playerScores[player] >= 0);
			playerMeeples[player] -= *m;
			Q_ASSERT(playerMeeples[player] >= 0);
		}
	}
}

bool Game::equals(Game const & other) const
{
	if (active != other.active)
		return false;
	if (nextPlayer != other.nextPlayer)
		return false;
	if (getPlayerCount() != other.getPlayerCount())
		return false;
	if ((board == 0) != (other.board == 0))
		return false;
	if (tiles.size() != other.tiles.size())
		return false;
	if (discardedTiles.size() != other.discardedTiles.size())
		return false;
	if ((playerMeeples == 0) != (other.playerMeeples == 0))
		return false;
	if ((returnMeeples == 0) != (other.returnMeeples == 0))
		return false;
	if ((playerScores == 0) != (other.playerScores == 0))
		return false;
	if (moveHistory.size() != other.moveHistory.size())
		return false;
	if (tileSets != other.tileSets)
		return false;
	if (board != 0 && !board->equals(*other.board))
		return false;
	for (int i = 0; i < tiles.size(); ++i)
		if (!tiles[i]->equals(*other.tiles[i], this))
			return false;
	for (size_t i = 0; i < discardedTiles.size(); ++i)
		if (!discardedTiles[i]->equals(*other.discardedTiles[i], this))
			return false;
	if (playerMeeples != 0)
	{
		for (int i = 0; i < getPlayerCount(); ++i)
		{
			if (playerMeeples[i] != other.playerMeeples[i])
				return false;
			if (returnMeeples[i] != other.returnMeeples[i])
				return false;
			if (playerScores[i] != other.playerScores[i])
				return false;
		}
	}
	for (size_t i = 0; i < moveHistory.size(); ++i)
		if (moveHistory[i] != other.moveHistory[i])
			return false;
	
	return true;
}

void Game::cleanUp()
{
	moveHistory.clear();
	
	delete board;
	qDeleteAll(tiles);
	tiles.clear();
	qDeleteAll(discardedTiles);
	discardedTiles.clear();
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
