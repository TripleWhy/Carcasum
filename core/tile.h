#ifndef TILE_H
#define TILE_H

#include <unordered_set>
#include <QList>
#include <QDebug>

#define DEBUG_IDS 1

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
	friend class DeepCloner;
public:
	typedef std::unordered_set<Tile const *> TilesType;

	TerrainType t;
	TilesType tiles;
private:
	uchar * const meeples; // Number of meeples on this node per player.
	uchar maxMeples = 0;
#if DEBUG_IDS
	friend class jcz::TileFactory;
	uint id = -1;
#endif
protected:
	Node * p = 0;
	int connections = 0;

public:
	Node(TerrainType t, Tile const * parent, Game const * g);
	virtual ~Node();

	inline void addMeeple(int player, Game * g)
	{
		if (p)
			return p->addMeeple(player, g);
		if (++meeples[player] > maxMeples)
			maxMeples = meeples[player];
		checkClose(g);
	}
	void removeMeeple(int player, Game * g);
	inline bool isOccupied() const
	{
		if (p)
			return p->isOccupied();
		return maxMeples;
	}
	inline uchar getMaxMeeples() const
	{
		if (p)
			return p->getMaxMeeples();
		return maxMeples;
	}
	inline uchar const * getMeeples() const
	{
		if (p)
			return p->getMeeples();
		return meeples;
	}
//	inline int getId()
//	{
//		if (p)
//			return p->getId();
//		return id;
//	}
	inline Node * getTarget()
	{
		if (p)
			return p->getTarget();
		return this;
	}
	inline Node * getTargetButNot(Node * nope)
	{
		if (p && p != nope)
			return p->getTargetButNot(nope);
		return this;
	}

protected:
	bool connect2(Node * n, Game * g);
	bool disconnect2(Node * n, Game * g);
public:
	virtual void connect(Node * n, Game * g) = 0;
	virtual void disconnect(Node * n, Game * g) = 0;
	virtual void checkClose(Game * g) = 0;
	virtual void checkUnclose(Game * g) = 0;
	virtual uchar getScore() = 0;
	virtual Node * clone(Tile const * parent, Game const * g) const = 0;
	
	virtual bool equals(Node const & other, Game const * g) const;
};

struct CityNode : public Node
{
private:
	uchar open;
	uchar bonus;

public:
	CityNode(Tile const * parent, Game const * g, uchar open, uchar bonus = 0)
		: Node(City, parent, g),
		  open(open),
		  bonus(bonus)
	{}

	virtual void connect(Node * n, Game * g);
	virtual void disconnect(Node * n, Game * g);
	virtual void checkClose(Game * g);
	virtual void checkUnclose(Game * g);
	inline virtual uchar getScore()
	{
		if (p)
			return p->getScore();
		return tiles.size() + bonus;
	}
	virtual Node * clone(Tile const * parent, Game const * g) const
	{
		if (p)
			return p->clone(parent, g);
		return new CityNode(parent, g, open, bonus);
	}
	inline bool isClosed() const
	{
		if (p)
			return static_cast<CityNode *>(p)->isClosed();
		return open == 0;
	}
	
	virtual bool equals(Node const & other, Game const * g) const
	{
		CityNode const * c = dynamic_cast<CityNode const *>(&other);
		if (c == 0)
			return false;
		if (!Node::equals(other, g))
			return false;
		return (open == c->open) && (bonus == c->bonus);
	}
};

struct FieldNode : public Node
{
	std::vector<CityNode *> cities;

	FieldNode(Tile const * parent, Game const * g)
		: Node(Field, parent, g)
	{}

	virtual void connect(Node * n, Game * g);
	virtual void disconnect(Node * n, Game * g);
	virtual void checkClose(Game * /*g*/) {}
	virtual void checkUnclose(Game * /*g*/) {}
	inline virtual uchar getScore()
	{
		if (p)
			return p->getScore();
		std::unordered_set<Node *> closedCities;
		for (CityNode * c : cities)
			if (c->isClosed())
				closedCities.insert(c->getTarget());
		return closedCities.size() * 3;
	}
	virtual Node * clone(Tile const * parent, Game const * g) const
	{
		if (p)
			return p->clone(parent, g);
		return new FieldNode(parent, g);
	}
	
	virtual bool equals(Node const & other, Game const * g) const
	{
		FieldNode const * f = dynamic_cast<FieldNode const *>(&other);
		if (f == 0)
			return false;
		if (!Node::equals(other, g))
			return false;
		return cities.size() == f->cities.size();
	}
};

struct RoadNode : public Node
{
private:
	uchar open;

public:
	RoadNode(Tile const * parent, Game const * g, uchar open)
		: Node(Road, parent, g),
		  open(open)
	{}

	virtual void connect(Node * n, Game * g);
	virtual void disconnect(Node * n, Game * g);
	virtual void checkClose(Game * g);
	virtual void checkUnclose(Game * g);
	inline virtual uchar getScore()
	{
		if (p)
			return p->getScore();
		return tiles.size();
	}
	virtual Node * clone(Tile const * parent, Game const * g) const
	{
		if (p)
			return p->clone(parent, g);
		return new RoadNode(parent, g, open);
	}
	
	virtual bool equals(Node const & other, Game const * g) const
	{
		RoadNode const * r = dynamic_cast<RoadNode const *>(&other);
		if (r == 0)
			return false;
		if (!Node::equals(other, g))
			return false;
		return (open == r->open);
	}
};

struct CloisterNode : public Node
{
private:
	uchar surroundingTiles;
	
public:
	CloisterNode(Tile const * parent, Game const * g, uchar surroundingTiles = 1)
	    : Node(Cloister, parent, g),
	      surroundingTiles(surroundingTiles)
	{}
	
	virtual void connect(Node * /*n*/, Game * /*g*/) { Q_ASSERT(false); }
	virtual void disconnect(Node * /*n*/, Game * /*g*/) { Q_ASSERT(false); }
	virtual void checkClose(Game * g);
	virtual void checkUnclose(Game * g);
	inline virtual uchar getScore()
	{
		if (p)
			return p->getScore();
		return surroundingTiles;
	}
	virtual Node * clone(Tile const * parent, Game const * g) const
	{
		if (p)
			return p->clone(parent, g);
		return new CloisterNode(parent, g, surroundingTiles);
	}
	inline void addSurroundingTile(Game * g)
	{
		if (p)
			return static_cast<CloisterNode *>(p)->addSurroundingTile(g);
		++surroundingTiles;
		if (isOccupied())
			checkClose(g);
	}
	inline void removeSurroundingTile(Game * g)
	{
		if (p)
			return static_cast<CloisterNode *>(p)->removeSurroundingTile(g);
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

class Tile
{
	friend class jcz::TileFactory;
	friend class DeepCloner;

public:
	enum Side { left = 0, up = 1, right = 2, down = 3 };
	enum TileSet { BaseGame = 0 };
	Q_DECLARE_FLAGS(TileSets, TileSet)
	typedef Node * EdgeType;

private:
	TerrainType edges[4];    // array of edge types
	uchar nodeCount = 0;     // length of nodes
	Node ** nodes = 0;       // array of node pointers
	EdgeType * edgeNodes[4]; // 4 arrays of edge connectors
	CloisterNode * cloister = 0;
#if DEBUG_IDS
	uint id = -1;
#endif

public:
	Side orientation = left;

	//Not neccessarily the best place (because this will need to be copied also):
	TileSet const tileSet;
	uchar const tileType;

private:
	Tile(TileSet tileSet, uchar tileType);

public:
	Tile(TileSet tileSet, uchar tileType, TerrainType const edges[4]);
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
	Tile&& operator=(Tile&& t) = delete;
	Tile& operator= (Tile const& t) = delete;

private:
	EdgeType * getEdgeNodes(Side side);
	void setEdgeNode(Side side, uchar index, Node * n);
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
	inline uchar getNodeCount() const { return nodeCount; }
	inline Node const * const * getCNodes() const { return nodes; }
	inline Node * const * getNodes() { return nodes; }

	void printSides(Node * n);
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
