#include "game.h"
#include "board.h"
#include "player.h"
#include "jcz/tilefactory.h"

#include <QVarLengthArray>

Game::Game(Random * r)
    : r(r)
{
	if (r == 0)
		this->r = new RandomTable();
}

Game::~Game()
{
	cleanUp();
	delete r;
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
		playerMeeples[i] = MEEPLE_COUNT;
	
	active = true;
	nextPlayer = 0;
	this->tileSets = tileSets;
	tiles = tileFactory->createPack(tileSets, this);
#if USE_RESET
	originalTiles = tiles;
#endif
	board = new Board(this, tiles.size());
	board->setStartTile(tiles.takeFirst());
	for (Tile * t : tiles)
	{
		while (tileCount.size() <= t->tileType)
			tileCount.append(0);
		++tileCount[t->tileType];
	}
	assertTileCount();

	for (uint i = 0; i < allPlayers.size(); ++i)
		allPlayers[i]->newGame(-1, this);
	for (uint i = 0; i < players.size(); ++i)
		players[i]->newGame(i, this);

#if !USE_RESET
	this->tileFactory = tileFactory;
#endif
}

void Game::newGame(Tile::TileSets tileSets, jcz::TileFactory * tileFactory, const std::vector<MoveHistoryEntry> & history)
{
	newGame(tileSets, tileFactory);
	if (getPlayerCount())
		applyHistory(history);
}

void Game::restartGame()
{
#if USE_RESET
	moveHistory.clear();
	discardedTiles.clear();
	tiles = originalTiles;
	
	int const playerCount = getPlayerCount();
	memset(playerScores, 0, sizeof(*playerScores) * playerCount);
	memset(returnMeeples, 0, sizeof(*returnMeeples) * playerCount);
	for (int i = 0; i < playerCount; ++i)
		playerMeeples[i] = MEEPLE_COUNT;
	
	active = true;
	nextPlayer = 0;
	board->reset();
	for (Tile * t : tiles)
		t->reset(this);

	board->setStartTile(tiles.takeFirst());
#else
	moveHistory.clear();
	qDeleteAll(tiles);
	tiles.clear();
	qDeleteAll(discardedTiles);
	discardedTiles.clear();

	int const playerCount = getPlayerCount();
	memset(playerScores, 0, sizeof(*playerScores) * playerCount);
	memset(returnMeeples, 0, sizeof(*returnMeeples) * playerCount);
	for (int i = 0; i < playerCount; ++i)
		playerMeeples[i] = MEEPLE_COUNT;

	active = true;
	nextPlayer = 0;
	tiles = tileFactory->createPack(tileSets, this);
	board->clear();
	board->setStartTile(tiles.takeFirst());
#endif
	for (int & c : tileCount)
		c = 0;
	for (Tile * t : tiles)
		++tileCount[t->tileType];
	assertTileCount();
}


void Game::restartGame(const std::vector<MoveHistoryEntry> & history)
{
	restartGame();
	applyHistory(history);
}

void Game::addPlayer(Player * player)
{
	if (!isFinished() || players.size() >= MAX_PLAYERS)
		return;
	players.push_back(player);
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

//void Game::setPlayers(QList<Player *> newPlayers)
//{
//	if (!isFinished() || newPlayers.size() > MAX_PLAYERS)
//		return;
//	clearPlayers();
//	players = newPlayers;
//	for (Player * p : newPlayers)
//		addWatchingPlayer(p);
//	playerCount = players.size();
//}

void Game::clearPlayers()
{
	if (!isFinished())
		return;
	for (auto it = allPlayers.begin(); it < allPlayers.end(); )
	{
		if (std::find(players.begin(), players.end(), *it) != players.end())
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

bool Game::step()
{
	if (isFinished())
		return false;
#if PRINT_STEPS
	qDebug() << "STEP:";
#endif
#if WATCH_SCORES
	std::vector<int> oldScores;
	for (uint i = 0; i < getPlayerCount(); ++i)
		oldScores.push_back(playerScores[i]);
#endif

	MoveHistoryEntry entry;
	entry.tile = nextTile();
	Tile * tile = tiles[entry.tile];
	TileMovesType && placements = board->getPossibleTilePlacements(tile);
	if (placements.isEmpty())
	{
		discardedTiles.push_back(tile);
#if PRINT_STEPS
 #if DEBUG_IDS
		qDebug() << (moveHistory.size() + 1) << "discarged Tile" << tile->id << "total:" << discardedTiles.size();
 #else
		qDebug() << (moveHistory.size() + 1) << "discarged Tile  type" << tile->tileType << "total:" << discardedTiles.size();
 #endif
#endif
		moveHistory.push_back(entry);
		tiles.removeAt(entry.tile);
		--tileCount[tile->tileType];
		assertTileCount();
		
		if (tiles.isEmpty())
		{
			endGame();
			return false;
		}
		else
		{
			return true;
		}
	}
	
//	qDebug() << "tile type:" << tile->tileType << "possible placements:" << placements.size();
	
	int const playerIndex = nextPlayer;
	Player * player = players[playerIndex];
	
	Move & move = entry.move;
	TileMove & tileMove = move.tileMove;
	for (int i = 0; i < 10; ++i)
	{
		tileMove = player->getTileMove(playerIndex, tile, entry, placements);
		if (std::find(placements.cbegin(), placements.cend(), tileMove) != placements.cend())
			break;
		else
		{
			qWarning("Player returned invalid move!");
			Q_ASSERT_X(false, "Game::step()", "Player returned invalid tile move!");
			endGame();
			return false;
		}
	}
//	qDebug() << (moveHistory.size() + 1) << "player" << playerIndex << tileMove.x << tileMove.y << tileMove.orientation;
	moveTile(tile, tileMove);
	
	MeepleMove & meepleMove = move.meepleMove;
	if (playerMeeples[playerIndex] > 0)
	{
		MeepleMovesType && possibleMeeples = getPossibleMeeplePlacements(tile);
		if (possibleMeeples.size() > 1)
		{
			for (int i = 0; i < 10; ++i)
			{
				meepleMove = player->getMeepleMove(playerIndex, tile, entry, possibleMeeples);
				if (std::find(possibleMeeples.cbegin(), possibleMeeples.cend(), meepleMove) != possibleMeeples.cend())
					break;
				else
				{
					qWarning("Player returned invalid meeple move!");
					Q_ASSERT_X(false, "Game::step()", "Player returned invalid meeple move!");
					endGame();
					return false;
				}
			}
			moveMeeple(tile, playerIndex, meepleMove);
//			qDebug() << (moveHistory.size() + 1) << "player" << playerIndex << "placed meeple on" << n->t;
//			qDebug() << (moveHistory.size() + 1) << "player" << playerIndex << "placed no meeple";
		}
	}
	returnMeeplesToPlayers();
	setNextPlayer();

#if PRINT_STEPS || !defined(QT_NO_DEBUG) || defined(QT_FORCE_ASSERTS)
	auto && unscoredMeeples = board->countUnscoredMeeples();
#endif
#if PRINT_STEPS
	qDebug() << "next player:" << nextPlayer;
	for (uint i = 0; i < getPlayerCount(); ++i)
		qDebug() << moveHistory.size() + 1 << "player" << i << "score:" << playerScores[i] << "meeples:" << playerMeeples[i] << "+" << unscoredMeeples[i] << "=" << (playerMeeples[i]+unscoredMeeples[i]);
	qDebug() << (moveHistory.size() + 1) << "player" << playerIndex << tileMove.x << tileMove.y << tileMove.orientation;
	if (!meepleMove.isNull())
		qDebug() << (moveHistory.size() + 1) << "player" << playerIndex << "placed meeple on" << tile->getNodes()[meepleMove.nodeIndex]->getTerrain();
	else
		qDebug() << (moveHistory.size() + 1) << "player" << playerIndex << "placed no meeple";
	qDebug();
#endif
#if !defined(QT_NO_DEBUG) || defined(QT_FORCE_ASSERTS)
	for (uint i = 0; i < getPlayerCount(); ++i)
		Q_ASSERT((playerMeeples[i]+unscoredMeeples[i]) == MEEPLE_COUNT);
#endif
#if WATCH_SCORES
	for (uint i = 0; i < getPlayerCount(); ++i)
		Q_ASSERT(playerScores[i] >= oldScores[i]);
#endif

	moveHistory.push_back(entry);
	for (Player * p : allPlayers)
		p->playerMoved(playerIndex, tile, entry);

	tiles.removeAt(entry.tile);
	--tileCount[tile->tileType];
	assertTileCount();
	if (tiles.isEmpty())
	{
		endGame();
		return false;
	}
	else
		return true;
}

bool Game::simStep(Player * player)
{
#if WATCH_SCORES
	std::vector<int> oldScores;
	for (uint i = 0; i < getPlayerCount(); ++i)
		oldScores.push_back(playerScores[i]);
#endif
	{
		MoveHistoryEntry entry;
		entry.tile = nextTile();
		Tile * tile = tiles[entry.tile];
		TileMovesType && placements = board->getPossibleTilePlacements(tile);
		if (placements.isEmpty())
		{
			discardedTiles.push_back(tile);
		}
		else
		{
			int const playerIndex = nextPlayer;
			Move & move = entry.move;
			TileMove & tileMove = move.tileMove;
			tileMove = player->getTileMove(playerIndex, tile, entry, placements);
			moveTile(tile, tileMove);

			if (playerMeeples[playerIndex] > 0)
			{
				MeepleMovesType && possibleMeeples = getPossibleMeeplePlacements(tile);
				if (possibleMeeples.size() > 1)
				{
					MeepleMove & meepleMove = move.meepleMove;
					meepleMove = player->getMeepleMove(playerIndex, tile, entry, possibleMeeples);
					moveMeeple(tile, playerIndex, meepleMove);
				}
			}
			setNextPlayer();
		}
		tiles.removeAt(entry.tile);
		--tileCount[tile->tileType];
		assertTileCount();
		moveHistory.push_back(std::move(entry));
	}
#if !defined(QT_NO_DEBUG) || defined(QT_FORCE_ASSERTS)
	returnMeeplesToPlayers();
	auto && unscoredMeeples = board->countUnscoredMeeples();
	for (uint i = 0; i < getPlayerCount(); ++i)
		Q_ASSERT((playerMeeples[i]+unscoredMeeples[i]) == MEEPLE_COUNT);
#endif
#if WATCH_SCORES
	for (uint i = 0; i < getPlayerCount(); ++i)
		Q_ASSERT(playerScores[i] >= oldScores[i]);
#endif

	return simCheckEndGame();
}

bool Game::simStep(int tileIndex, const TileMove & tileMove, int playerIndex, Player * player)
{
	Q_ASSERT(!tileMove.isNull());
#if WATCH_SCORES
	std::vector<int> oldScores;
	for (uint i = 0; i < getPlayerCount(); ++i)
		oldScores.push_back(playerScores[i]);
#endif
	{
		MoveHistoryEntry entry;
		entry.tile = tileIndex;
		Tile * tile = tiles[entry.tile];

		Move & move = entry.move;
		move.tileMove = tileMove;
		moveTile(tile, tileMove);

		if (playerMeeples[playerIndex] > 0)
		{
			MeepleMovesType && possibleMeeples = getPossibleMeeplePlacements(tile);
			if (possibleMeeples.size() > 1)
			{
				MeepleMove & meepleMove = move.meepleMove;
				meepleMove = player->getMeepleMove(playerIndex, tile, entry, possibleMeeples);
				moveMeeple(tile, playerIndex, meepleMove);
			}
		}
		setNextPlayer();

		tiles.removeAt(entry.tile);
		--tileCount[tile->tileType];
		assertTileCount();
		moveHistory.push_back(std::move(entry));
	}
	

#if WATCH_SCORES
		for (uint i = 0; i < getPlayerCount(); ++i)
			Q_ASSERT(playerScores[i] >= oldScores[i]);
#endif

	return simCheckEndGame();
}

bool Game::simStep(const MoveHistoryEntry & entry)
{
#if WATCH_SCORES
	std::vector<int> oldScores;
	for (uint i = 0; i < getPlayerCount(); ++i)
		oldScores.push_back(playerScores[i]);
#endif
	
	Tile * tile = tiles[entry.tile];
	if (entry.move.tileMove.isNull())
	{
		discardedTiles.push_back(tile);
	}
	else
	{
		Move const & move = entry.move;
		moveTile(tile, move.tileMove);
		moveMeeple(tile, nextPlayer, move.meepleMove);
		setNextPlayer();
	}
	moveHistory.push_back(entry);
	tiles.removeAt(entry.tile);
	--tileCount[tile->tileType];
	assertTileCount();

#if WATCH_SCORES
	for (uint i = 0; i < getPlayerCount(); ++i)
		Q_ASSERT(playerScores[i] >= oldScores[i]);
#endif

	return simCheckEndGame();
}

void Game::undo()
{
#if WATCH_SCORES
	std::vector<int> oldScores;
	for (uint i = 0; i < getPlayerCount(); ++i)
		oldScores.push_back(playerScores[i]);
#endif
	Q_ASSERT(moveHistory.size() > 0);

	if (tiles.isEmpty())
	{
		board->unscoreEndGame();
		active = true;
	}
	
	{
		MoveHistoryEntry const & entry = moveHistory.back();
		Move const & move = entry.move;
		TileMove const & tileMove = move.tileMove;

		if (tileMove.isNull())
		{
			Tile * t = discardedTiles.back();
			discardedTiles.pop_back();
			tiles.insert(entry.tile, t);
			++tileCount[t->tileType];
			assertTileCount();

#if PRINT_STEPS
			qDebug() << (moveHistory.size() + 1) << "discarged Tile" << t->tileType << "total:" << discardedTiles.size();
#endif

			moveHistory.pop_back();
//			if (moveHistory.size() > 0)
//				undo();
//			else
//				return;	// just to be able to place a debug point here
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
			n->removeMeeple(playerIndex, this);
			Q_ASSERT(playerMeeples[playerIndex] <= MEEPLE_COUNT);
		}

		board->removeTile(tileMove);

		tiles.insert(entry.tile, tile);
		++tileCount[tile->tileType];
		assertTileCount();
	}
	moveHistory.pop_back();

#if PRINT_STEPS || !defined(QT_NO_DEBUG) || defined(QT_FORCE_ASSERTS)
	auto && unscoredMeeples = board->countUnscoredMeeples();
#endif
#if PRINT_STEPS
	qDebug() << (moveHistory.size() + 1) << "player" << playerIndex << tileMove.x << tileMove.y << tileMove.orientation;
	if (!meepleMove.isNull())
		qDebug() << (moveHistory.size() + 1) << "player" << playerIndex << "placed meeple on" << tile->getNodes()[meepleMove.nodeIndex]->getTerrain();
	else
		qDebug() << (moveHistory.size() + 1) << "player" << playerIndex << "placed no meeple";

	qDebug();
	qDebug() << "UNDO:";
	qDebug() << "next player:" << nextPlayer;

	for (uint i = 0; i < getPlayerCount(); ++i)
		qDebug() << (moveHistory.size()) << "player" << i << "score:" << playerScores[i] << "meeples:" << playerMeeples[i] << "+" << unscoredMeeples[i] << "=" << (playerMeeples[i]+unscoredMeeples[i]);
#endif
#if !defined(QT_NO_DEBUG) || defined(QT_FORCE_ASSERTS)
	for (uint i = 0; i < getPlayerCount(); ++i)
		Q_ASSERT((playerMeeples[i]+unscoredMeeples[i]) == MEEPLE_COUNT);
#endif
#if WATCH_SCORES
		for (uint i = 0; i < getPlayerCount(); ++i)
			Q_ASSERT(playerScores[i] <= oldScores[i]);
#endif
}

void Game::cityClosed(CityNode * n)
{
	int score = n->getScore();
	if (score > 2)
		score *= 2;
//	qDebug() << "   city closed, value:" << score;
	
	scoreNodeMidGame(n, score);
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
	scoreNodeMidGame(n, score);
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
	
	scoreNodeMidGame(n, score);
}

void Game::cloisterUnclosed(CloisterNode * n)
{
	static int const score = 9;
	unscoreNode(n, score);
}

void Game::scoreNodeEndGame(Node * n)
{
	if (n->getScored() != NotScored)
		return;
	int const score = n->getScore();
//	if (score != 0)
	{
		scoreNode(n, score);
		n->setScored(ScoredEndGame);
	}
}

void Game::scoreNodeMidGame(Node * n, const int score)
{
	scoreNode(n, score);
	n->setScored(ScoredMidGame);
}

void Game::unscoreNodeEndGame(Node * n)
{
	if (n->getScored() != ScoredEndGame)
		return;
	
	int const score = n->getScore();
//	if (score != 0)
		unscoreNode(n, score);
//	else
//	{
//		Q_ASSERT(false);
//		n->setScored(NotScored);
//	}
}

void Game::scoreNode(Node * n, int const score)
{
	Q_ASSERT(score >= 0);
	uchar meepleCount = n->getMaxMeeples();
	int player = 0;
	for (uchar const * m = n->getMeeples(), * end = m + getPlayerCount(); m < end; ++m, ++player)
	{
		if (*m == meepleCount)
			playerScores[player] += score;
		returnMeeples[player] += *m;
	}
	
//	qDebug() << "Player scores:";
//	for (uint i = 0; i < getPlayerCount(); ++i)
	//		qDebug() << "  Player" << i << ":" << playerScores[i];
}

void Game::unscoreNode(Node * n, const int score)
{
	Q_ASSERT(score >= 0);
	uchar meepleCount = n->getMaxMeeples();
	int player = 0;
	for (uchar const * m = n->getMeeples(), * end = m + getPlayerCount(); m < end; ++m, ++player)
	{
		if (*m == meepleCount)
		{
			playerScores[player] -= score;
			Q_ASSERT(playerScores[player] >= 0);
		}
		playerMeeples[player] -= *m;
		Q_ASSERT(playerMeeples[player] >= 0);
	}
	n->setScored(NotScored);
}

void Game::simEndGame()
{
	board->scoreEndGame();
	returnMeeplesToPlayers();
	active = false;

#if PRINT_STEPS || !defined(QT_NO_DEBUG) || defined(QT_FORCE_ASSERTS)
	auto && unscoredMeeples = board->countUnscoredMeeples();
#endif
#if PRINT_STEPS
	qDebug("GAME OVER:");
	for (uint i = 0; i < getPlayerCount(); ++i)
		qDebug() << (moveHistory.size() + 1) << "player" << i << "score:" << playerScores[i] << "meeples:" << playerMeeples[i] << "+" << unscoredMeeples[i] << "=" << (playerMeeples[i]+unscoredMeeples[i]);
#endif
#if !defined(QT_NO_DEBUG) || defined(QT_FORCE_ASSERTS)
	for (uint i = 0; i < getPlayerCount(); ++i)
		Q_ASSERT((playerMeeples[i]+unscoredMeeples[i]) == MEEPLE_COUNT);
#endif
}

void Game::endGame()
{
	simEndGame();
	for (Player * p : allPlayers)
		p->endGame();
}

void Game::moveTile(Tile * tile, const TileMove & tileMove)
{
	board->addTile(tile, tileMove);
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
	if (tileCount != other.tileCount)
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
	for (int i = 0; i < tiles.size(); ++i)
		if (!tiles[i]->equals(*other.tiles[i], this))
			return false;
	for (size_t i = 0; i < discardedTiles.size(); ++i)
		if (!discardedTiles[i]->equals(*other.discardedTiles[i], this))
			return false;
	if (playerMeeples != 0)
	{
		for (uint i = 0; i < getPlayerCount(); ++i)
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
	if (board != 0 && !board->equals(*other.board))
		return false;
	
	return true;
}

void Game::cleanUp()
{
	moveHistory.clear();
	
	delete board;
	qDeleteAll(tiles);
	tiles.clear();
#if USE_RESET
	originalTiles.clear();
#endif
	qDeleteAll(discardedTiles);
	discardedTiles.clear();
	tileCount.clear();
	delete[] playerScores;
	delete[] playerMeeples;
	delete[] returnMeeples;
}

void Game::applyHistory(const std::vector<MoveHistoryEntry> & history)
{
	for (MoveHistoryEntry const & e : history)
		simStep(e);
	returnMeeplesToPlayers();
}
