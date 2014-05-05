#include "game.h"
#include "board.h"
#include "player.h"
#include "jcz/tilefactory.h"
#include <QFile>

Game::Game(NextTileProvider * ntp)
    : ntp(ntp)
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
		playerMeeples[i] = MEEPLE_COUNT;
	
	active = true;
	nextPlayer = 0;
	this->tileSets = tileSets;
	tiles = tileFactory->createPack(tileSets, this);
	upperScoreBound = calcUpperScoreBound(tiles);
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

void Game::newGame(Tile::TileSets tileSets, jcz::TileFactory * tileFactory, const std::vector<MoveHistoryEntry> & history, bool informPlayers)
{
	newGame(tileSets, tileFactory);
	if (getPlayerCount())
		applyHistory(history, informPlayers);
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
	playerCount = (uint)players.size();
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

TileMovesType Game::getPossibleTilePlacements(Tile const * tile) const
{
	return board->getPossibleTilePlacements(tile);
}

bool Game::step()
{
#if CHECK_SIM_STATE
	Q_ASSERT(simState == 0);
#endif
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
	entry.tileIndex = nextTile();
	Tile * tile = tiles[entry.tileIndex];
	entry.tileType = tile->tileType;
	TileMovesType && placements = board->getPossibleTilePlacements(tile);
	if (placements.size() == 0)
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
		tiles.removeAt(entry.tileIndex);
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
//			Q_ASSERT_X(false, "Game::step()", "Player returned invalid tile move!");
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

	tiles.removeAt(entry.tileIndex);
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
	Q_ASSERT(!isFinished());
#if CHECK_SIM_STATE
	Q_ASSERT(simState == 0);
#endif
#if WATCH_SCORES
	std::vector<int> oldScores;
	for (uint i = 0; i < getPlayerCount(); ++i)
		oldScores.push_back(playerScores[i]);
#endif
	{
		MoveHistoryEntry entry;
		entry.tileIndex = simNextTile();
		Tile * tile = tiles[entry.tileIndex];
		entry.tileType = tile->tileType;
		TileMovesType && placements = board->getPossibleTilePlacements(tile);
		if (placements.size() == 0)
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
		tiles.removeAt(entry.tileIndex);
		--tileCount[tile->tileType];
		assertTileCount();
		moveHistory.push_back(std::move(entry));
	}
#if !defined(QT_NO_DEBUG) || defined(QT_FORCE_ASSERTS)
	returnMeeplesToPlayers();
	auto && unscoredMeeples = board->countUnscoredMeeples();
	for (uint i = 0; i < getPlayerCount(); ++i)
	{
//		qDebug() << i << ":" << playerMeeples[i] << "+" << unscoredMeeples[i] << "+" << returnMeeples[i] << "=" << (playerMeeples[i]+unscoredMeeples[i]+returnMeeples[i]);
		Q_ASSERT_X((playerMeeples[i]+unscoredMeeples[i]) == MEEPLE_COUNT, "Game::simStep(Player * player)", QString("Player meeples incorrect: player %1: meeples %2, unscored: %3, return: %4)").arg(i).arg(playerMeeples[i]).arg(unscoredMeeples[i]).arg(returnMeeples[i]).toStdString().c_str());
		Q_ASSERT((playerMeeples[i]+unscoredMeeples[i]+returnMeeples[i]) == MEEPLE_COUNT);
	}
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
#if CHECK_SIM_STATE
	Q_ASSERT(simState == 0);
#endif
#if WATCH_SCORES
	std::vector<int> oldScores;
	for (uint i = 0; i < getPlayerCount(); ++i)
		oldScores.push_back(playerScores[i]);
#endif
	{
		MoveHistoryEntry entry;
		entry.tileIndex = tileIndex;
		Tile * tile = tiles[entry.tileIndex];
		entry.tileType = tile->tileType;

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

		tiles.removeAt(entry.tileIndex);
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
#if CHECK_SIM_STATE
	Q_ASSERT(simState == 0);
#endif
#if WATCH_SCORES
	std::vector<int> oldScores;
	for (uint i = 0; i < getPlayerCount(); ++i)
		oldScores.push_back(playerScores[i]);
#endif
	
	Tile * tile = tiles[entry.tileIndex];
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
	tiles.removeAt(entry.tileIndex);
	--tileCount[tile->tileType];
	assertTileCount();

#if WATCH_SCORES
	for (uint i = 0; i < getPlayerCount(); ++i)
		Q_ASSERT(playerScores[i] >= oldScores[i]);
#endif

	return simCheckEndGame();
}

void Game::simPartStepChance(int index)
{
#if CHECK_SIM_STATE
	Q_ASSERT(simState++ == 0);
#endif

	simTile = tiles[index];
	simEntry.tileIndex = index;
	simEntry.tileType = simTile->tileType;
}

void Game::simPartStepTile(TileMove const & tileMove)
{
#if CHECK_SIM_STATE
	Q_ASSERT(simState++ == 1);
#endif
	Q_ASSERT(simTile != 0);

	simEntry.move.tileMove = tileMove;
	if (tileMove.isNull())
	{
		discardedTiles.push_back(simTile);
	}
	else
	{
		moveTile(simTile, tileMove);
	}

	assertMeepleCount();
}

void Game::simPartStepMeeple(const MeepleMove & meepleMove)
{
#if CHECK_SIM_STATE
	Q_ASSERT(simState == 2);
	simState = 0;
#endif

	Q_ASSERT(simTile != 0);
	Q_ASSERT(meepleMove.isNull() || playerMeeples[nextPlayer] > 0);
#if !defined(QT_NO_DEBUG) || defined(QT_FORCE_ASSERTS)
	auto && possible = getPossibleMeeplePlacements(simTile);
	Q_ASSERT(std::find(possible.begin(), possible.end(), meepleMove) != possible.end());
#endif

	simEntry.move.meepleMove = meepleMove;
	if (!simEntry.move.tileMove.isNull())
	{
		moveMeeple(simTile, nextPlayer, meepleMove);
		returnMeeplesToPlayers();
		setNextPlayer();
	}

	tiles.removeAt(simEntry.tileIndex);
	--tileCount[simTile->tileType];
	moveHistory.push_back(std::move(simEntry));
	simEntry = MoveHistoryEntry();
#ifndef QT_NO_DEBUG
	simTile = 0;
#endif

	assertMeepleCount();

	if (tiles.isEmpty())
		endGame();
}

void Game::undo()
{
#if CHECK_SIM_STATE
	Q_ASSERT(simState == 0);
#endif
#if WATCH_SCORES
	std::vector<int> oldScores;
	for (uint i = 0; i < getPlayerCount(); ++i)
		oldScores.push_back(playerScores[i]);
#endif
	Q_ASSERT(moveHistory.size() > 0);

	if (tiles.isEmpty())
	{
		simUnEndGame();
	}
	
	{
		MoveHistoryEntry const & entry = moveHistory.back();
		Move const & move = entry.move;
		TileMove const & tileMove = move.tileMove;

		if (tileMove.isNull())
		{
			Tile * t = discardedTiles.back();
			discardedTiles.pop_back();
			tiles.insert(entry.tileIndex, t);
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

		tiles.insert(entry.tileIndex, tile);
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

void Game::simPartUndoChance()
{
#if CHECK_SIM_STATE
	Q_ASSERT(simState-- == 1);
#endif

	simEntry.tileIndex = -1;
	simEntry.tileType = -1;
#ifndef QT_NO_DEBUG
	simTile = 0;
#endif
}

void Game::simPartUndoTile()
{
#if CHECK_SIM_STATE
	Q_ASSERT(simState-- == 2);
#endif

	TileMove & tileMove = simEntry.move.tileMove;
	if (tileMove.isNull())
	{
		discardedTiles.pop_back();
	}
	else
	{
		board->removeTile(tileMove);
	}
	tileMove = TileMove();

	assertMeepleCount();
}

void Game::simPartUndoMeeple()
{
#if CHECK_SIM_STATE
	Q_ASSERT(simState == 0);
	simState = 2;
#endif

	if (tiles.isEmpty())
	{
		simUnEndGame();
	}

	simEntry = moveHistory.back();
	moveHistory.pop_back();
	Move & move = simEntry.move;
	TileMove const & tileMove = move.tileMove;

	Tile * tile;
	if (tileMove.isNull())
	{
		tile = discardedTiles.back();
	}
	else
	{
		tile = board->getTile(tileMove);

		nextPlayer = (nextPlayer - 1 + getPlayerCount()) % getPlayerCount();
		int const playerIndex = nextPlayer;
		MeepleMove & meepleMove = move.meepleMove;

		if (!meepleMove.isNull())
		{
			auto nodes = tile->getNodes();
			Node * n = nodes[meepleMove.nodeIndex];
			++playerMeeples[playerIndex];
			n->removeMeeple(playerIndex, this);

			meepleMove = MeepleMove();
		}
	}
	simTile = tile;

	tiles.insert(simEntry.tileIndex, tile);
	++tileCount[tile->tileType];
	assertTileCount();
	assertMeepleCount();
}

void Game::storeToFile(const QString & path, const std::vector<MoveHistoryEntry> & history)
{
	QFile file(path);
	file.open(QIODevice::WriteOnly | QIODevice::Truncate);

	QDataStream out(&file);

	// file format version
	out << (quint32)1;

	// history size
	out << (quint32)history.size();

	for (MoveHistoryEntry const & e : history)
	{
		out << (qint32)e.tileIndex;
		out << (qint32)e.tileType;

		Move const & move = e.move;
		TileMove const & tileMove = move.tileMove;
		out << (quint32)tileMove.x;
		out << (quint32)tileMove.y;
		out << (quint8)tileMove.orientation;

		MeepleMove const & meepleMove = move.meepleMove;
		out << (quint8)meepleMove.nodeIndex;
	}

	file.close();
}

std::vector<MoveHistoryEntry> Game::loadFromFile(const QString & path)
{
	std::vector<MoveHistoryEntry> history;

	QFile file(path);
	file.open(QIODevice::ReadOnly);

	QDataStream in(&file);

	// file format version
	quint32 version;
	in >> version;
	if (version != 1)
		return history;

	// history size
	quint32 size;
	in >> size;

	history.resize(size);

	for (quint32 i = 0; i < size; ++i)
	{
		qint32 i32;
		quint32 u32;
		quint8 u8;

		MoveHistoryEntry &e = history[i];
		in >> i32;
		e.tileIndex = (int)i32;
		in >> i32;
		e.tileType = (TileTypeType)i32;

		Move & move = e.move;
		TileMove & tileMove = move.tileMove;
		in >> u32;
		tileMove.x = (uint)u32;
		in >> u32;
		tileMove.y = (uint)u32;
		in >> u8;
		tileMove.orientation = (Tile::Side)u8;

		MeepleMove & meepleMove = move.meepleMove;
		in >> u8;
		meepleMove.nodeIndex = (uchar)u8;
	}

	file.close();
	return history;
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
	Q_ASSERT(n->getScored() == NotScored);
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

void Game::simUnEndGame()
{
	board->unscoreEndGame();
	active = true;
}

void Game::endGame()
{
	simEndGame();
	for (Player * p : allPlayers)
		p->endGame();
}

void Game::moveTile(Tile * tile, const TileMove & tileMove)
{
	Q_ASSERT(!tileMove.isNull());
	board->addTile(tile, tileMove);
}

int Game::calcUpperScoreBound(QList<Tile *> const & tiles)
{
	int roads = 0;
	int cities = 0;
	int tinyCities = 0;
	int cloisters = 0;
	for (Tile const * t : tiles)
	{
		auto nodes = t->getCNodes();
		for (int i = 0, c = t->getNodeCount(); i < c; ++i)
		{
			Node const * n = nodes[i];
			switch (n->getTerrain())
			{
				case Field:
					break;
				case Road:
					++roads;
					break;
				case City:
					++cities;
					if (static_cast<CityNode const *>(n)->getOpen() == 1)
						++tinyCities;
					break;
				case Cloister:
					++cloisters;
					break;
				case None:
					break;
			}
		}
	}

	return ((tinyCities / 2) * 3) + (cities * 2) + (roads) + (cloisters * 9);
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

void Game::applyHistory(const std::vector<MoveHistoryEntry> & history, bool informPlayers)
{
	if (informPlayers)
	{
		for (MoveHistoryEntry const & e : history)
		{
			int playerIndex = nextPlayer;
			Tile const * tile = tiles[e.tileIndex];
			simStep(e);
			returnMeeplesToPlayers();
			for (Player * p : allPlayers)
				p->playerMoved(playerIndex, tile, e);
			if (tiles.isEmpty())
				endGame();
		}
	}
	else
	{
		for (MoveHistoryEntry const & e : history)
			simStep(e);
		returnMeeplesToPlayers();
	}
}

#if !defined(QT_NO_DEBUG) || defined(QT_FORCE_ASSERTS)
void Game::assertMeepleCount()
{
	auto && unscoredMeeples = board->countUnscoredMeeples();
	for (uint i = 0; i < getPlayerCount(); ++i)
//		Q_ASSERT_X((playerMeeples[i]+unscoredMeeples[i]+returnMeeples[i]) == MEEPLE_COUNT, "Game::assertMeepleCount()", QString("Player meeples incorrect: player %1: meeples %2, unscored: %3, return: %4)").arg(i).arg(playerMeeples[i]).arg(unscoredMeeples[i]).arg(returnMeeples[i]).toStdString().c_str());
		Q_ASSERT((playerMeeples[i]+unscoredMeeples[i]+returnMeeples[i]) == MEEPLE_COUNT);
}
#endif
