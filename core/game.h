#ifndef GAME_H
#define GAME_H

#include "static.h"
#include "tile.h"
#include "util.h"

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
	
	MeepleMove(uchar index = -1) : nodeIndex(index) {}
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

class Game
{
public:

private:
//	int ply = -1;
	int active = false;
	int nextPlayer = -1;
	QList<Player *> players;
	std::vector<Player *> allPlayers;
	Board * board = 0;
	QList<Tile *> tiles;
	QList<Tile *> originalTiles;
	std::vector<Tile *> discardedTiles;
	Random r;
	int * playerMeeples = 0;
	int * returnMeeples = 0;
	int * playerScores = 0;
	int playerCount = 0;
	
	std::vector<MoveHistoryEntry> moveHistory;
	Tile::TileSets tileSets;
#if !USE_RESET
	jcz::TileFactory * tileFactory;
#endif

public:
	Game();
	~Game();

	void newGame(Tile::TileSets tileSets, jcz::TileFactory * tileFactory);
	void newGame(Tile::TileSets tileSets, jcz::TileFactory * tileFactory, std::vector<MoveHistoryEntry> const & history);
	void restartGame();
	void restartGame(std::vector<MoveHistoryEntry> const & history);
	void addPlayer(Player * player);
	void addWatchingPlayer(Player * player);
//	void setPlayer(int index, Player * player);
	void setPlayers(QList<Player *> players);
	void clearPlayers();
	Board const * getBoard() const;
	void step();
	void step(int tileIndex, TileMove const & tileMove, int playerIndex, Player * player);
	void step(MoveHistoryEntry const & entry);
	void undo();

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
	bool equals(const Game & other) const;

	inline bool isFinished() const { return !active; }
	inline int getPlayerCount() const { return playerCount; }
	inline std::vector<MoveHistoryEntry> const & getMoveHistory() const { return moveHistory; }
	inline Tile::TileSets const & getTileSets() const { return tileSets; }
	inline int const * getScores() const { return playerScores; }
//	inline int getPly() const { return ply; }

private:
	inline void returnMeeplesToPlayers()
	{
		for (int * r = returnMeeples, * end = r + getPlayerCount(), * m = playerMeeples; r < end; ++r, ++m)
		{
			*m += *r;
			*r = 0;
		}
	}

private:
	void cleanUp();
	void applyHistory(std::vector<MoveHistoryEntry> const & history);
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
