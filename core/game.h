#ifndef GAME_H
#define GAME_H

#include "tile.h"
#include "board.h"
#include "util.h"

class Player;
class Tile;
class Node;

#define NODE_ARRAY_LENGTH 16

struct TileMove
{
	uint x, y;
	Tile::Side orientation;

	TileMove() noexcept : x(-1) {}
	TileMove(uint x, uint y, Tile::Side orientation) noexcept : x(x), y(y), orientation(orientation) {}

	inline bool isNull()
	{
		return x == (uint)-1;
	}
};

typedef Node const * MeepleMove; // TODO This will need to change. If I want to replay a game on a different object, pointers won't work.

class Game : public QObject
{
//Q_OBJECT

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

public:
	Game();
	~Game();

	void newGame(Tile::TileSets tileSets, jcz::TileFactory * tileFactory);
	void addPlayer(Player * player);
	void addWatchingPlayer(Player * player);
//	void setPlayer(int index, Player * player);
	void setPlayers(QList<Player *> players);
	void clearPlayers();
	Board const * getBoard() const;
	bool isFinished();
	void step();
	//getWinner()
	//undo(...)

	void cityClosed(CityNode * n);
	void roadClosed(RoadNode * n);
	void scoreNode(Node * n, const int score);

	inline int getPlayerCount() const { return playerCount; }

private:
	void cleanUp();

//signals:
//	void boardChanged(Board const * board);
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
