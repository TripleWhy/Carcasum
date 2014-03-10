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
	};

	Game * game;
	Tile *** board;
	uint const size;
//	uint const offset;
	QHash<QPoint, TerrainWrapper> open; //TODO ist is unsorted_map faster? Should I maybe use a custom point type?
	std::unordered_map<CloisterNode *, QPoint> openCloisters;

public:
	Board(Game * game, uint const size);
	~Board();

	Tile * getTile(uint x, uint y);
	const Tile * getTile(uint x, uint y) const;
	void setStartTile(Tile  * tile);
	void addTile(Tile * tile, const TileMove & move);
	uint getInternalSize() const;

	TileMovesType getPossibleTilePlacements(Tile const * tile) const;
	QList<QPoint> getOpenPlaces() const;

	QPoint positionOf(Tile * t) const;
	
	inline void cloisterClosed(CloisterNode * n) { openCloisters.erase(n); }
	void scoreEndGame();
};

#endif // BOARD_H
