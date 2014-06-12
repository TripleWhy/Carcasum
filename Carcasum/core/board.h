#ifndef BOARD_H
#define BOARD_H

#include "tile.h"
#include "game.h"
#include "player.h"
#include <QHash>
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wconversion"
#include <QPoint>
#pragma GCC diagnostic pop

class Game;
struct TileMove;

inline uint qHash(const QPoint &p)
{
	return (p.x() << (sizeof(p.x()) * 8 / 2)) | (p.y() & ((1 << (sizeof(p.x()) * 8 / 2)) - 1));
}

struct EdgeMask
{
	TerrainType t[4];
	EdgeMask() : t{None, None, None, None} {}
	EdgeMask(TerrainType left, TerrainType up, TerrainType right, TerrainType down)
	{
		t[Tile::left] = left;
		t[Tile::up] = up;
		t[Tile::right] = right;
		t[Tile::down] = down;
	}

	inline bool operator==(EdgeMask const& rhs) const { return (t[0] == rhs.t[0]) && (t[1] == rhs.t[1]) && (t[2] == rhs.t[2]) && (t[3] == rhs.t[3]); }
	inline bool operator!=(EdgeMask const& rhs) const { return !operator==(rhs); }
};

class Board
{
public:

private:
	Game * game;
	Tile *** board;
	uint const size;
	QHash<QPoint, EdgeMask> open; //TODO This is the only place in the core, where I use Qt data structures. Is unsorted_map faster? Should I maybe use a custom point type?
	std::vector<Tile *> tiles;

public:
	Board(Game * game, uint const size);
	~Board();

	void setStartTile(Tile  * tile);
	void addTile(Tile * tile, const TileMove & move);
	void removeTile(const TileMove & move);
	uint getInternalSize() const;
	void clear();
	void reset();

	TileMovesType getPossibleTilePlacements(Tile const * tile) const;
	QList<QPoint> getOpenPlaces() const;

	QPoint positionOf(Tile const * t) const;
	
	void scoreEndGame();
	void unscoreEndGame();
	
	std::vector<int> countUnscoredMeeples() const;
	bool equals(Board const & other) const;
	
	inline Tile * getTile(uint x, uint y ){ return board[x][y]; }
	inline const Tile * getTile(uint x, uint y) const { return board[x][y]; }
	inline Tile * getTile(TileMove const & m) { return board[m.x][m.y]; }
	inline const Tile * getTile(TileMove const & m) const { return board[m.x][m.y]; }
	inline Tile * getTile(QPoint const & p) { return board[p.x()][p.y()]; }
	inline const Tile * getTile(QPoint const & p) const { return board[p.x()][p.y()]; }
	inline std::vector<Tile *> const & getTiles() { return tiles; }
	inline std::vector<Tile const *> const & getTiles() const { return *reinterpret_cast<std::vector<Tile const *> const *>(&tiles); }	//Dirty solution. I don't know a better one.

	EdgeMask getEdgeMask(QPoint const & position) const { return open[position]; }
	EdgeMask getEdgeMask(uint x, uint y) const { return getEdgeMask(QPoint(x, y)); }

	int getOffset() const { return size / 2; }
};

#endif // BOARD_H
