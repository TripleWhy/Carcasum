#ifndef GAME_H
#define GAME_H

#include "static.h"
#include "tile.h"
#include "util.h"
#include "player.h"
#include "util.h"
#include "random.h"
#include "nexttileprovider.h"

class Player;
class Tile;
class Node;
class Board;

struct TileMove
{
	uint x, y;
	Tile::Side orientation;

	TileMove() noexcept : x(-1) {}
	TileMove(uint x, uint y, Tile::Side orientation) noexcept : x(x), y(y), orientation(orientation) {}

	inline bool isNull() const
	{
		return x == (uint)-1;
	}
};

struct MeepleMove
{
	uchar nodeIndex;
	
	constexpr MeepleMove(uchar index = -1) : nodeIndex(index) {}
	inline bool isNull() const { return nodeIndex == (uchar)-1; }
};

struct Move
{
	TileMove tileMove;
	MeepleMove meepleMove;
};

struct MoveHistoryEntry
{
	int tile;
	Move move;
};

typedef QVarLengthArray<int, TILE_COUNT_ARRAY_LENGTH> TileCountType;

class Game
{
private:
//	int ply = -1;
	int active = false;
	int nextPlayer = -1;
	std::vector<Player *> players;
	std::vector<Player *> allPlayers;
	Board * board = 0;
	QList<Tile *> tiles;
	TileCountType tileCount;
#if USE_RESET
	QList<Tile *> originalTiles;
#endif
	std::vector<Tile *> discardedTiles;
	DefaultRandom r;
	NextTileProvider * ntp;
	int * playerMeeples = 0;
	int * returnMeeples = 0;
	int * playerScores = 0;
	uint playerCount = 0;

	uint upperScoreBound = 0;
	
	std::vector<MoveHistoryEntry> moveHistory;
	Tile::TileSets tileSets;
#if !USE_RESET
	jcz::TileFactory * tileFactory;
#endif
#if CHECK_SIM_STATE
	int simState = 0;
#endif

public:
	MoveHistoryEntry simEntry;
	Tile * simTile = 0;

public:
	Game(NextTileProvider * ntp);
	~Game();

	void newGame(Tile::TileSets tileSets, jcz::TileFactory * tileFactory);
	void newGame(Tile::TileSets tileSets, jcz::TileFactory * tileFactory, std::vector<MoveHistoryEntry> const & history, bool informPlayers = false);
	void restartGame();
	void restartGame(std::vector<MoveHistoryEntry> const & history);
	void addPlayer(Player * player);
	void addWatchingPlayer(Player * player);
//	void setPlayer(int index, Player * player);
//	void setPlayers(QList<Player *> players);
	void clearPlayers();
	Board const * getBoard() const;
	TileMovesType getPossibleTilePlacements(const Tile * tile) const;
	bool step();
	bool simStep(Player * player);
	bool simStep(int tileIndex, TileMove const & tileMove, int playerIndex, Player * player);
	bool simStep(MoveHistoryEntry const & entry);
	void simPartStepChance(int index);
	void simPartStepTile(const TileMove & tileMove);
	void simPartStepMeeple(const MeepleMove & meepleMove);
	void undo();
	void simPartUndoChance();
	void simPartUndoTile();
	void simPartUndoMeeple();
	static void storeToFile(QString const & path, std::vector<MoveHistoryEntry> const & history);
	static std::vector<MoveHistoryEntry> loadFromFile(QString const & path);


	void cityClosed(CityNode * n);
	void cityUnclosed(CityNode * n);
	void roadClosed(RoadNode * n);
	void roadUnclosed(RoadNode * n);
	void cloisterClosed(CloisterNode * n);
	void cloisterUnclosed(CloisterNode * n);
	void scoreNodeEndGame(Node * n);
	void scoreNodeMidGame(Node * n, const int score);
	void unscoreNodeEndGame(Node * n);
private:
	void scoreNode(Node * n, const int score);
	void unscoreNode(Node * n, const int score);
public:
	void simEndGame();
	void simUnEndGame();
private:
	inline void endGame();
	inline void moveTile(Tile * tile, TileMove const & tileMove);
	int calcUpperScoreBound(const QList<Tile *> & tiles);
	
public:
	bool equals(const Game & other) const;

	inline bool isFinished() const { return !active; }
	inline bool isTerminal() const { return tiles.isEmpty(); }
	inline uint getPlayerCount() const { return playerCount; }
	inline std::vector<MoveHistoryEntry> const & getMoveHistory() const { return moveHistory; }
	inline Tile::TileSets const & getTileSets() const { return tileSets; }
	inline int const * getScores() const { return playerScores; }
	inline int getPlayerMeeples(int player) const { return playerMeeples[player]; }
	inline int getPlayerScore(int player) const { return playerScores[player]; }
	inline int getTileCount() const { return tiles.size(); }
	inline TileCountType const & getTileCounts() const { return tileCount; }
	inline QList<Tile *> const & getTiles() const { return tiles; }
	inline std::vector<Tile *> const & getDiscargedTiles() const { return discardedTiles; }
	inline void setNextTileProvider(NextTileProvider * n) { ntp = n; }
	inline std::vector<Player *> const & getPlayers() const { return players; }
	inline int getNextPlayer() const { return nextPlayer; }
	inline uint getUpperScoreBound() const { return upperScoreBound; }

	inline MeepleMovesType getPossibleMeeplePlacements(Tile const * tile) const
	{
		MeepleMovesType possibleMeeples(1);
//		possibleMeeples[0] = MeepleMove();	// Not neccessary for non-primitive types

		auto nodes = tile->getNodes();
		for (uchar i = 0, end = tile->getNodeCount(); i < end; ++i)
			if (Util::isNodeFree(nodes[i]))
				possibleMeeples.push_back(MeepleMove(i));
		Q_ASSERT_X(possibleMeeples.size() <= NODE_ARRAY_LENGTH, "Game::step()", "possibleMeeples initial size too low");
		return possibleMeeples;
	}
	inline MeepleMovesType getPossibleMeeplePlacements(int player, Tile const * tile) const
	{
		if (getPlayerMeeples(player) > 0)
			return getPossibleMeeplePlacements(tile);
		else
			return MeepleMovesType(1);
	}

	inline int getTileIndexByType(int tileType) const
	{
		int index = 0;
		for (int i = 0; i < tileType; ++i)
			index += tileCount[i];
		Q_ASSERT(tiles[index]->tileType == tileType && (index == 0 || tiles[index-1]->tileType != tileType));
		return index;
	}

	inline Tile const * getTileByType(int tileType) const
	{
		return tiles[getTileIndexByType(tileType)];
	}

private:
	inline void setNextPlayer() { nextPlayer = (nextPlayer + 1) % getPlayerCount(); }
	inline int simNextTile() { return r.nextInt(tiles.size()); }
	inline int nextTile() { return ntp->nextTile(this); }

	inline void returnMeeplesToPlayers()
	{
		for (int * r = returnMeeples, * end = r + getPlayerCount(), * m = playerMeeples; r < end; ++r, ++m)
		{
			*m += *r;
			*r = 0;
		}
	}
	inline void moveMeeple(Tile * tile, int playerIndex, MeepleMove const & meepleMove)
	{
		if (!meepleMove.isNull())
		{
			Node * n = tile->getNode(meepleMove.nodeIndex);
			n->addMeeple(playerIndex, this);
			--playerMeeples[playerIndex];
		}
	}
	inline bool simCheckEndGame()
	{
		if (tiles.isEmpty())
		{
			simEndGame();
			return false;
		}
		else
		{
			returnMeeplesToPlayers();
			return true;
		}
	}
	inline void assertTileCount()
	{
#if !defined(QT_NO_DEBUG) || defined(QT_FORCE_ASSERTS)
		int sum = 0;
		for (int c : tileCount)
			sum += c;
		Q_ASSERT(sum == tiles.size());
#endif
	}

private:
	void cleanUp();
	void applyHistory(std::vector<MoveHistoryEntry> const & history, bool informPlayers = false);

#if !defined(QT_NO_DEBUG) || defined(QT_FORCE_ASSERTS)
	void assertMeepleCount();
#else
 #define assertMeepleCount();
#endif
};

inline bool operator==(TileMove const& lhs, TileMove const& rhs) { return (lhs.x == rhs.x) && (lhs.y == rhs.y) && (lhs.orientation == rhs.orientation); }
inline bool operator!=(TileMove const& lhs, TileMove const& rhs) { return !(lhs == rhs); }
inline bool operator==(MeepleMove const& lhs, MeepleMove const& rhs) { return (lhs.nodeIndex == rhs.nodeIndex); }
inline bool operator!=(MeepleMove const& lhs, MeepleMove const& rhs) { return !(lhs == rhs); }
inline bool operator==(Move const& lhs, Move const& rhs) { return (lhs.tileMove == rhs.tileMove) && (lhs.meepleMove == rhs.meepleMove); }
inline bool operator!=(Move const& lhs, Move const& rhs) { return !(lhs == rhs); }
inline bool operator==(MoveHistoryEntry const& lhs, MoveHistoryEntry const& rhs) { return (lhs.tile == rhs.tile) && (lhs.move == rhs.move); }
inline bool operator!=(MoveHistoryEntry const& lhs, MoveHistoryEntry const& rhs) { return !(lhs == rhs); }


#endif // GAME_H
