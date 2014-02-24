#ifndef GAME_H
#define GAME_H

#include "player.h"
#include "tile.h"
#include "board.h"
#include "util.h"

class Player;

struct Move
{
	uint x, y;
	Tile::Side orientation;
	//TODO meeple move

	Move() noexcept : x(-1) {}
	Move(uint x, uint y, Tile::Side orientation) noexcept : x(x), y(y), orientation(orientation) {}

	inline bool isNull()
	{
		return x == (uint)-1;
	}
};

class Game : public QObject
{
Q_OBJECT

private:
	int ply;
	QList<Player *> players;
	Board * board;
	QList<Tile *> tiles;
	Random r;

public:
	Game();
	~Game();

	void newGame(Tile::TileSets tileSets, JCZUtils::TileFactory * tileFactory);
	void addPlayer(Player * player);
	void setPlayer(int index, Player * player);
	void setPlayers(QList<Player *> players);
	void clearPlayers();
	Board const * getBoard() const;
	bool isFinished();
	void step();
	//getWinner()
	//undo(...)

	static void test();

private:
	void cleanUp();

signals:
	void boardChanged(Board const * board);
};


#endif // GAME_H
