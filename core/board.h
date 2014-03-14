#ifndef BOARD_H
#define BOARD_H

#include "tile.h"
#include "util.h"
#include "player.h"

#include <QHash>
#include <QPoint>

#include <unordered_map>

class Game;
struct TileMove;

inline uint qHash(const QPoint &p)
{
	return (p.x() << (sizeof(p.x()) * 8 / 2)) | (p.y() & ((1 << (sizeof(p.x()) * 8 / 2)) - 1));
}

class Board
{
private:
	struct TerrainWrapper
	{
		TerrainType t[4];
		TerrainWrapper() : t{None, None, None, None} {}
		TerrainWrapper(TerrainType left, TerrainType up, TerrainType right, TerrainType down)
		{
			t[Tile::left] = left;
			t[Tile::up] = up;
			t[Tile::right] = right;
			t[Tile::down] = down;
		}
		
		inline bool operator==(TerrainWrapper const& rhs) const { return (t[0] == rhs.t[0]) && (t[1] == rhs.t[1]) && (t[2] == rhs.t[2]) && (t[3] == rhs.t[3]); }
		inline bool operator!=(TerrainWrapper const& rhs) const { return !operator==(rhs); }
	};

	Game * game;
	Tile *** board;
	uint const size;
//	uint const offset;
	QHash<QPoint, TerrainWrapper> open; //TODO ist is unsorted_map faster? Should I maybe use a custom point type?

public:
	Board(Game * game, uint const size);
	~Board();

	void setStartTile(Tile  * tile);
	void addTile(Tile * tile, const TileMove & move);
	void removeTile(const TileMove & move);
	uint getInternalSize() const;
	void clear();

	TileMovesType getPossibleTilePlacements(Tile const * tile) const;
	QList<QPoint> getOpenPlaces() const;

	QPoint positionOf(Tile * t) const;
	
	void scoreEndGame();
	void unscoreEndGame();
	
	bool equals(Board const & other) const;
	
	inline Tile * getTile(uint x, uint y ){ return board[x][y]; }
	inline const Tile * getTile(uint x, uint y) const { return board[x][y]; }
	inline Tile * getTile(TileMove const & m) { return board[m.x][m.y]; }
	inline const Tile * getTile(TileMove const & m) const { return board[m.x][m.y]; }
};

#endif // BOARD_H
