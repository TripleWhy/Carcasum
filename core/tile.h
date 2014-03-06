#ifndef TILE_H
#define TILE_H

#include <unordered_set>
#include <QList>
#include <QDebug>

#define NODE_VARIANT 0

namespace jcz
{
class TileFactory;
}
class Tile;
class Game;

enum TerrainType { None = 0, Field, City, Road, Cloister };

struct Node
{
	friend class Tile;
	friend class Game;

public:
	TerrainType t;
	std::unordered_set<Tile *> tiles;
private:
	QList<Node **> pointers;
	uchar * const meeples; // Number of meeples on this node per player.
	uchar maxMeples = 0;

public:
	Node(TerrainType t, Tile * parent, Game const * g);
	virtual ~Node();

	inline void addMeeple(int player, Game * g) { if (++meeples[player] > maxMeples) maxMeples = meeples[player]; checkClose(g); }
	inline bool isOccupied() const { return maxMeples; }
	inline uchar getMaxMeeples() const { return maxMeples; }
	inline uchar const * getMeeples() const { return meeples; }

	virtual void connect(Node * n, Game * /*g*/);
	virtual void checkClose(Game * /*g*/) {}
	virtual Node * clone(Tile * parent, Game const * g) const
	{
		return new Node(t, parent, g);
	}
protected:
	inline void addPointer(Node ** p) { pointers.push_back(p); }
};

struct CityNode : public Node
{
	uchar open;
	uchar bonus;

public:
	CityNode(Tile * parent, Game const * g, uchar open, uchar bonus = 0)
		: Node(City, parent, g),
		  open(open),
		  bonus(bonus)
	{}

	virtual void connect(Node * n, Game * g);
	virtual void checkClose(Game * g);

	virtual Node * clone(Tile * parent, Game const * g) const
	{
		return new CityNode(parent, g, open, bonus);
	}
};

struct RoadNode : public Node
{
	uchar open;

public:
	RoadNode(Tile * parent, Game const * g, uchar open)
		: Node(Road, parent, g),
		  open(open)
	{}

	virtual void connect(Node * n, Game * g);
	virtual void checkClose(Game * g);

	virtual Node * clone(Tile * parent, Game const * g) const
	{
		return new RoadNode(parent, g, open);
	}
};

class Tile
{
	friend class jcz::TileFactory;

public:
	enum Side { left = 0, up = 1, right = 2, down = 3 };
	enum TileSet { BaseGame = 0 };
	Q_DECLARE_FLAGS(TileSets, TileSet)
#if NODE_VARIANT
	typedef Node ** EdgeType;
#else
	typedef Node * EdgeType;
#endif



private:
	TerrainType edges[4];    // array of edge types
	int nodeCount;           // lenght of nodes
	Node ** nodes;           // array of node pointers
	EdgeType * edgeNodes[4]; // 4 arrays of edge connectors

public:
	Side orientation = left;

	//Not neccessarily the best place (because this will need to be copied also):
	TileSet const tileSet;
	int const tileType;

private:
	Tile(TileSet tileSet, int tileType);

public:
	Tile(TileSet tileSet, int tileType, TerrainType const edges[4], const int nodeCount, Node ** nodes);
	Tile(const Tile & t) = delete; // I don't want implicit copies. Use clone() instead.
	Tile (Tile&& t) = delete; // I don't actually want this to happen.
	~Tile();
	TerrainType const & getEdge(Side side) const;
	TerrainType const & getEdge(Side side, Side orientation) const;
	bool connect(Side s, Tile * other, Game * game);
	Tile * clone(Game const * g);
	Tile&& operator=(Tile&& t) = delete;
	Tile& operator= (Tile const& t) = delete;

private:
	EdgeType * getEdgeNodes(Side side);
	void setEdgeNode(Side side, int index, Node * n);
	void createEdgeList(Side side);

	static inline int edgeNodeCount(TerrainType t)
	{
		switch (t)
		{
			case Field:
			case City:
				return 1;
			case Road:
				return 3;
			case None:
			case Cloister:
				break;
		}
		return 0;
	}

public:
	inline int getNodeCount() const { return nodeCount; }
	inline Node const * const * getCNodes() const { return nodes; }
	inline Node * const * getNodes() { return nodes; }

	void printSides(Node * n);
};
Q_DECLARE_OPERATORS_FOR_FLAGS(Tile::TileSets)

inline QDebug operator<< (QDebug d, const Tile::Side & s)
{
	switch (s)
	{
		case Tile::left:
			d << "left";
			break;
		case Tile::up:
			d << "up";
			break;
		case Tile::right:
			d << "right";
			break;
		case Tile::down:
			d << "down";
			break;
	}
	return d;
}

inline QDebug operator<< (QDebug d, const TerrainType & t)
{
	switch (t)
	{
		case None:
			d << "none";
			break;
		case Field:
			d << "field";
			break;
		case City:
			d << "city";
			break;
		case Road:
			d << "road";
			break;
		case Cloister:
			d << "cloister";
			break;
	}
	return d;
}

#endif // TILE_H
