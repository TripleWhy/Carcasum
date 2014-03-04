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

typedef Node const * MeepleMove;

class Game : public QObject
{
Q_OBJECT

private:
	int ply;
	int currentPlayer;
	QList<Player *> players;
	Board * board;
	QList<Tile *> tiles;
	Random r;

public:
	Game();
	~Game();

	void newGame(Tile::TileSets tileSets, jcz::TileFactory * tileFactory);
	void addPlayer(Player * player);
	void setPlayer(int index, Player * player);
	void setPlayers(QList<Player *> players);
	void clearPlayers();
	Board const * getBoard() const;
	bool isFinished();
	void step();
	//getWinner()
	//undo(...)

	void cityClosed(CityNode * n);
	void roadClosed(RoadNode * n);

private:
	void cleanUp();

signals:
	void boardChanged(Board const * board);
};


#endif // GAME_H
