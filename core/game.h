#ifndef GAME_H
#define GAME_H

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
	int ply = -1;
	int currentPlayer = -1;
	QList<Player *> players;
	std::vector<Player *> allPlayers;
	Board * board = 0;
	QList<Tile *> tiles;
	Random r;
	int * playerMeeples = 0;
	int * returnMeeples = 0;
	int * playerScores = 0;
	int playerCount = 0;
	
	std::vector<MoveHistoryEntry> moveHistory;
	Tile::TileSets tileSets;

public:
	Game();
	~Game();

	void newGame(Tile::TileSets tileSets, jcz::TileFactory * tileFactory);
	void newGame(Tile::TileSets tileSets, jcz::TileFactory * tileFactory, std::vector<MoveHistoryEntry> const & history);
	void restartGame(jcz::TileFactory * tileFactory);
	void restartGame(jcz::TileFactory * tileFactory, std::vector<MoveHistoryEntry> const & history);
	void addPlayer(Player * player);
	void addWatchingPlayer(Player * player);
//	void setPlayer(int index, Player * player);
	void setPlayers(QList<Player *> players);
	void clearPlayers();
	Board const * getBoard() const;
	bool isFinished();
	void step();
	void step(int tileIndex, TileMove const & tileMove, int playerIndex, Player * player);
	void step(MoveHistoryEntry const & entry);
	//getWinner()
	//undo(...)

	void cityClosed(CityNode * n);
	void roadClosed(RoadNode * n);
	void cloisterClosed(CloisterNode * n);
	void scoreNode(Node * n, const int score);

	inline int getPlayerCount() const { return playerCount; }
	inline std::vector<MoveHistoryEntry> const & getMoveHistory() const { return moveHistory; }
	inline Tile::TileSets const & getTileSets() const { return tileSets; }
	inline int const * getScores() const { return playerScores; }
	inline int getPly() const { return ply; }

private:
	void cleanUp();
	void applyHistory(std::vector<MoveHistoryEntry> const & history);
};

inline bool operator==(TileMove const& lhs, TileMove const& rhs)
{
	return (lhs.x == rhs.x) && (lhs.y == rhs.y) && (lhs.orientation == rhs.orientation);
}

inline bool operator!=(TileMove const& lhs, TileMove const& rhs)
{
	return !(lhs == rhs);
}


#endif // GAME_H
