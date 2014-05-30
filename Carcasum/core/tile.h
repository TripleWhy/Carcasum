#ifndef TILE_H
#define TILE_H

#include "static.h"
#include <unordered_set>

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wconversion"
#include <QList>
#include <QDebug>
#pragma GCC diagnostic pop

namespace jcz
{
class TileFactory;
}
class Tile;
class Game;

#define TERRAIN_TYPE_SIZE 5
enum TerrainType { None = 0, Field, City, Road, Cloister };
enum Scored { NotScored, ScoredMidGame, ScoredEndGame };
typedef quint8 TileTypeType;

struct Node
{
	friend class Tile;
#if DEBUG_IDS
	friend class jcz::TileFactory;
#endif
public:
	typedef std::vector<Node **>                  PointersType;
	typedef std::unordered_multiset<Tile const *> TilesType;

	struct NodeData
	{
		TerrainType t;
		TilesType tiles;
//		PointersType pointers;
		uchar * const meeples; // Number of meeples on this node per player.
		uchar maxMeples = 0;
		void * dd = 0;
		Scored scored = NotScored;
		std::unordered_multiset<Node *> nodes; // This may actually also work faster in a linked version.
#if DEBUG_IDS
		uint id = -1;
#endif
		
		inline NodeData(Node * parent, int const playerCount) : meeples(new uchar[playerCount]()) { nodes.insert(parent); }
		inline ~NodeData() { delete[] meeples; }
	};
private:
	NodeData data;
#if DEBUG_IDS
public:
	inline int id() const { return d-> id; }
	inline int realId() const { return data.id; }
#endif
protected:
	NodeData * d = &data;
	std::vector<NodeData *> ds = { d };
	inline bool isSame (Node const * n) { return d == n->d; }

public:
	Node(TerrainType t, Tile const * parent, Game const * g);
	virtual ~Node();

	void removeMeeple(int player, Game * g);
	inline bool isOccupied() const { return d->maxMeples; }
	inline uchar getMaxMeeples() const { return d->maxMeples; }
	inline uchar const * getMeeples() const { return d->meeples; }
	inline uchar getPlayerMeeples(int player) const { return d->meeples[player]; }
	inline TerrainType getTerrain() const { return d->t; }
	inline Scored getScored() const { return d->scored; }
	inline void setScored(Scored s) { d->scored = s; }
	inline NodeData const * getData() const { return d; }

	virtual void connect(Node * n, Game * g);
	virtual void disconnect(Node * n, Game * g);
	virtual void checkClose(Game * g) = 0;
	virtual void checkUnclose(Game * g) = 0;
	virtual int getScore() const = 0;
	virtual Node * clone(Tile const * parent, Game const * g) const = 0;
#if USE_RESET
	virtual void reset(Tile const * parent, Game const * g);
#endif
	
	inline void addMeeple(int player, Game * g)
	{
		if (++d->meeples[player] > d->maxMeples)
			d->maxMeples = d->meeples[player];
		checkClose(g);
	}
	inline uint uniqueTileCount() const
	{
		uint uniqueTiles = 0;
		Tile const * last = 0;
		for (Tile const * t : d->tiles)
		{
			if (t != last)
			{
				++uniqueTiles;
				last = t;
			}
		}
		return uniqueTiles;
	}
	
	virtual bool equals(Node const & other, Game const * g) const;
};

struct CityNode : public Node
{
private:
	struct CityNodeData
	{
		uchar open;
		uchar bonus;
	};
	CityNodeData cityData;
#if USE_RESET
	CityNodeData originalCityData;
#endif
	inline CityNodeData * getCityData() { return static_cast<CityNodeData *>(d->dd); }
	inline CityNodeData const * getCityData() const { return static_cast<CityNodeData *>(d->dd); }

public:
	CityNode(Tile const * parent, Game const * g, uchar open, uchar bonus = 0)
		: Node(City, parent, g)
	{
		d->dd = &cityData;
		cityData.open = open;
		cityData.bonus = bonus;
#if USE_RESET
		originalCityData = cityData;
#endif
	}

	virtual void connect(Node * n, Game * g);
	virtual void disconnect(Node * n, Game * g);
	virtual void checkClose(Game * g);
	virtual void checkUnclose(Game * g);
#if USE_RESET
	virtual void reset(Tile const * parent, Game const * g);
#endif
	inline virtual int getScore() const
	{
		return uniqueTileCount() + getCityData()->bonus;
	}
	virtual Node * clone(Tile const * parent, Game const * g) const
	{
		return new CityNode(parent, g, getCityData()->open, getCityData()->bonus);
	}
	inline bool isClosed() const
	{
		return getCityData()->open == 0;
	}
	inline int getOpen() const
	{
		return getCityData()->open;
	}
	
	virtual bool equals(Node const & other, Game const * g) const
	{
		CityNode const * c = dynamic_cast<CityNode const *>(&other);
		if (c == 0)
			return false;
		if (!Node::equals(other, g))
			return false;
		return (getCityData()->open == c->getCityData()->open) && (getCityData()->bonus == c->getCityData()->bonus);
	}
};

struct FieldNode : public Node
{
private:
	struct FieldNodeData
	{
		std::vector<CityNode *> cities;
	};
	FieldNodeData fieldData;
#if USE_RESET
friend class jcz::TileFactory;
	FieldNodeData originalFieldData;
#endif
public:
	inline FieldNodeData * getFieldData() { return static_cast<FieldNodeData *>(d->dd); }
	inline FieldNodeData const * getFieldData() const { return static_cast<FieldNodeData *>(d->dd); }

public:
	FieldNode(Tile const * parent, Game const * g)
		: Node(Field, parent, g)
	{
		d->dd = &fieldData;
	}

	virtual void connect(Node * n, Game * g);
	virtual void disconnect(Node * n, Game * g);
	virtual void checkClose(Game * /*g*/) {}
	virtual void checkUnclose(Game * /*g*/) {}
#if USE_RESET
	virtual void reset(Tile const * parent, Game const * g);
#endif
	inline virtual int getScore() const
	{
		std::unordered_set<NodeData const *> closedCities;
		for (CityNode * c : getFieldData()->cities)
			if (c->isClosed())
				closedCities.insert(c->getData());
		return (int)closedCities.size() * 3;
	}
	virtual Node * clone(Tile const * parent, Game const * g) const
	{
		return new FieldNode(parent, g);
	}
	
	virtual bool equals(Node const & other, Game const * g) const
	{
		FieldNode const * f = dynamic_cast<FieldNode const *>(&other);
		if (f == 0)
			return false;
		if (!Node::equals(other, g))
			return false;
		return getFieldData()->cities.size() == f->getFieldData()->cities.size();
	}
};

struct RoadNode : public Node
{
private:
	struct RoadNodeData
	{
		uchar open;
	};
	RoadNodeData roadData;
#if USE_RESET
	RoadNodeData originalRoadData;
#endif
	inline RoadNodeData * getRoadData() { return static_cast<RoadNodeData *>(d->dd); }
	inline RoadNodeData const * getRoadData() const { return static_cast<RoadNodeData *>(d->dd); }

public:
	RoadNode(Tile const * parent, Game const * g, uchar open)
		: Node(Road, parent, g)
	{
		d->dd = &roadData;
		roadData.open = open;
#if USE_RESET
		originalRoadData = roadData;
#endif
	}

	virtual void connect(Node * n, Game * g);
	virtual void disconnect(Node * n, Game * g);
	virtual void checkClose(Game * g);
	inline virtual int getScore() const { return uniqueTileCount(); }
	virtual void checkUnclose(Game * g);
#if USE_RESET
	virtual void reset(Tile const * parent, Game const * g);
#endif
	virtual Node * clone(Tile const * parent, Game const * g) const
	{
		return new RoadNode(parent, g, getRoadData()->open);
	}
	inline int getOpen() const
	{
		return getRoadData()->open;
	}
	
	virtual bool equals(Node const & other, Game const * g) const
	{
		RoadNode const * r = dynamic_cast<RoadNode const *>(&other);
		if (r == 0)
			return false;
		if (!Node::equals(other, g))
			return false;
		return (getRoadData()->open == r->getRoadData()->open);
	}
};

struct CloisterNode : public Node
{
private:
	uchar surroundingTiles;
	
public:
	CloisterNode(Tile const * parent, Game const * g)
	    : Node(Cloister, parent, g),
		  surroundingTiles(1)
	{}
	
	virtual void connect(Node * /*n*/, Game * /*g*/) { Q_ASSERT(false); }
	virtual void disconnect(Node * /*n*/, Game * /*g*/) { Q_ASSERT(false); }
	virtual void checkClose(Game * g);
	inline virtual int getScore() const { return surroundingTiles; }
	virtual void checkUnclose(Game * g);
#if USE_RESET
	virtual void reset(Tile const * parent, Game const * g);
#endif
	virtual Node * clone(Tile const * parent, Game const * g) const
	{
		return new CloisterNode(parent, g);
	}
	inline void addSurroundingTile(Game * g)
	{
		++surroundingTiles;
		if (isOccupied())
			checkClose(g);
	}
	inline void removeSurroundingTile(Game * g)
	{
		if (isOccupied())
			checkUnclose(g);
		--surroundingTiles;
	}
	
	virtual bool equals(Node const & other, Game const * g) const
	{
		CloisterNode const * c = dynamic_cast<CloisterNode const *>(&other);
		if (c == 0)
			return false;
		if (!Node::equals(other, g))
			return false;
		return (surroundingTiles == c->surroundingTiles);
	}
};

#define EDGE_NODE_COUNT 3
class Tile
{
	friend class jcz::TileFactory;

public:
	enum Side { left = 0, up = 1, right = 2, down = 3 };
	//TODO might be faster using switch
	static inline Side sideOpposite(Side const & side) { return (Tile::Side)((side + 2) % 4); }
	static inline Side sideRotateCW(Side const & side) { return (Tile::Side)((side + 1) % 4); }
	static inline Side sideRotateCCW(Side const & side) { return (Tile::Side)((side + 4 - 1) % 4); }

	enum TileSet { BaseGame = 1 << 0 };
//	struct TileType { TileSet set; int type; };
	Q_DECLARE_FLAGS(TileSets, TileSet)
#if NODE_VARIANT == 1
	typedef Node ** EdgeType;
#elif NODE_VARIANT == 0
	typedef Node * EdgeType;
#elif NODE_VARIANT == 2
	typedef qint8 EdgeType;
#endif


private:
	TerrainType edges[4];    // array of edge types
	uchar nodeCount = 0;     // length of nodes
	Node ** nodes = 0;       // array of node pointers
	EdgeType edgeNodes[4][EDGE_NODE_COUNT] = // 4 arrays of edge connectors
#if NODE_VARIANT == 2
	{{-1, -1, -1}, {-1, -1, -1}, {-1, -1, -1}, {-1, -1, -1}};
#else
	{};
#endif
	CloisterNode * cloister = 0;
#if DEBUG_IDS
public:
	uint id = -1;
#endif

public:
	Side orientation = left;

	//Not neccessarily the best place (because this will need to be copied also):
	TileTypeType tileType;

private:
	constexpr Tile(TileTypeType tileType)
	    : edges {None, None, None, None},
	      tileType(tileType) {}
public:
	constexpr Tile(TileTypeType tileType, TerrainType const edges[4])
	    : edges { edges[0], edges[1], edges[2], edges[3] },
	      tileType(tileType) {}
	Tile(const Tile & t) = delete; // I don't want implicit copies. Use clone() instead.
	Tile (Tile&& t) = delete; // I don't actually want this to happen.
	~Tile();
	TerrainType const & getEdge(Side side) const;
	TerrainType const & getEdge(Side side, Side orientation) const;
	void connect(Side side, Tile * other, Game * game);
	void disconnect(Side side, Tile * other, Game * game);
	void connectDiagonal(Tile * other, Game * game);
	void disconnectDiagonal(Tile * other, Game * game);
	Tile * clone(Game const * g);
#if USE_RESET
	void reset(const Game * g);
#endif
	Tile&& operator=(Tile&& t) = delete;
	Tile& operator= (Tile const& t) = delete;

private:
	EdgeType * getEdgeNodes(Side side);
	EdgeType * getEdgeNodes(Side side, Side orientation);
	EdgeType const * getEdgeNodes(Side side) const;
	EdgeType const * getEdgeNodes(Side side, Side orientation) const;
	void setEdgeNode(Side side, int index, Node * n);
	inline bool isNull(EdgeType const & et) const
	{
#if NODE_VARIANT == 2
		return et == -1;
#else
		return et == 0;
#endif
	}

public:
	inline uchar getNodeCount() const { return nodeCount; }
	inline Node const * const * getCNodes() const { return nodes; }
	inline Node * const * getNodes() { return nodes; }
	inline Node const * const * getNodes() const { return nodes; }
	inline Node * getNode(int idx) { return nodes[idx]; }
	inline Node const * getNode(int idx) const { return nodes[idx]; }
	inline Node const * getFeatureNode(Side const & side) const { return getEdgeNode(side, 1); }
	inline Node const * getFeatureNode(Side const & side, Side orientation) const { return getEdgeNode(side, 1, orientation); }
	inline Node const * getFeatureNodeAndIndex(Side const & side, uchar & nodeIndex) const { return getEdgeNodeAndIndex(side, 1, nodeIndex); }
	inline Node const * getFeatureNodeAndIndex(Side const & side, uchar & nodeIndex, Side orientation) const { return getEdgeNodeAndIndex(side, 1, nodeIndex, orientation); }
	inline Node const * getFieldNode(Side side, int index) const { return (getEdge(side) == Field) ? getFeatureNode(side) : getEdgeNode(side, index); }
	inline Node const * getFieldNode(Side side, int index, Side orientation) const { return (getEdge(side) == Field) ? getFeatureNode(side, orientation) : getEdgeNode(side, index, orientation); }
	inline Node const * getCloisterNode() const { return cloister; }

	inline Node const * getEdgeNode(Side side, int index) const
	{
		EdgeType const & e = getEdgeNodes(side)[index];
#if NODE_VARIANT == 0
		return e;
#elif NODE_VARIANT == 1
		if (isNull(e))
			return 0;
		return *e;
#elif NODE_VARIANT == 2
		if (isNull(e))
			return 0;
		return nodes[e];
#endif
	}
	inline Node const * getEdgeNode(Side side, int index, Side orientation) const
	{
		EdgeType const & e = getEdgeNodes(side, orientation)[index];
#if NODE_VARIANT == 0
		return e;
#elif NODE_VARIANT == 1
		if (isNull(e))
			return 0;
		return *e;
#elif NODE_VARIANT == 2
		if (isNull(e))
			return 0;
		return nodes[e];
#endif
	}

	inline Node const * getEdgeNodeAndIndex(Side side, int index, uchar & nodeIndex) const
	{
#if NODE_VARIANT == 0
		Node * n = getEdgeNodes(side)[index];
		nodeIndex = -1;
		for (uchar i = 0; i < nodeCount; ++i)
			if (nodes[i] == n)
			{
				nodeIndex = i;
				break;
			}
		return n;
#elif NODE_VARIANT == 1
		EdgeType p = getEdgeNodes(side)[index];
		Q_ASSERT(!isNull(p));
		if (isNull(p))
			nodeIndex = -1;
		else
			nodeIndex = p - nodes;
		return *p;
#elif NODE_VARIANT == 2
		return nodes[ (nodeIndex = getEdgeNodes(side)[index]) ];
#endif
	}

	inline Node const * getEdgeNodeAndIndex(Side side, int index, uchar & nodeIndex, Side orientation) const
	{
#if NODE_VARIANT == 0
		Node * n = getEdgeNodes(side, orientation)[index];
		nodeIndex = -1;
		for (uchar i = 0; i < nodeCount; ++i)
			if (nodes[i] == n)
			{
				nodeIndex = i;
				break;
			}
		return n;
#elif NODE_VARIANT == 1
		EdgeType p = getEdgeNodes(side, orientation)[index];
		Q_ASSERT(!isNull(p));
		if (isNull(p))
			nodeIndex = -1;
		else
			nodeIndex = p - nodes;
		return *p;
#elif NODE_VARIANT == 2
		return nodes[nodeIndex = getEdgeNodes(side, orientation)[index]];
#endif
	}

//	void printSides(Node * n);
	bool equals(Tile const & other, const Game * g) const;
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
