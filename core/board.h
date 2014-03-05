#ifndef BOARD_H
#define BOARD_H

#include "tile.h"
#include "util.h"

#include <QHash>
#include <QPoint>

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
	QHash<QPoint, TerrainWrapper> open;

public:
	Board(Game * game, uint const size);
	~Board();

	Tile * getTile(uint x, uint y);
	const Tile * getTile(uint x, uint y) const;
	void setStartTile(Tile  * tile);
	void addTile(uint x, uint y, Tile * tile);
	uint getInternalSize() const;

	QList<TileMove> getPossibleTilePlacements(Tile const * tile) const;
	QList<QPoint> getOpenPlaces() const;

	QPoint positionOf(Tile * t) const;
};

#endif // BOARD_H
