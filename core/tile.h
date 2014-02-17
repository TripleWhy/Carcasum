#ifndef TILE_H
#define TILE_H

#include <QList>
#include <QSet>
#include <QDebug>

#define NODE_VARIANT 0

enum TerrainType { None = 0, Field, City, Road, Cloister };

struct Node
{
	friend class Tile;

public:
	TerrainType t;
//	QSet<Node *> nodes;
//	const int id = nextId++;
//	static int nextId;
//	bool deleted = false;
private:
	QList<Node **> pointers;

public:
	Node(TerrainType t) : t(t) {}
	virtual ~Node() {}

	virtual void connect(Node * n)
	{
		if (this == n)
			return;

		foreach (Node ** p, n->pointers)
			*p = this;
		pointers.append(n->pointers);

//		n->deleted = true;
		delete n;
	}
};

struct CityNode : public Node
{
	uchar score;
	uchar open;

	CityNode(uchar open, uchar score = 1)
		: Node(City),
		  score(score),
		  open(open)
	{}

	virtual void connect(Node * n)
	{
		Q_ASSERT_X(n->t == this->t, "CityNode::connect", "TerrainType does not match");
		Q_ASSERT_X(dynamic_cast<CityNode *>(n) != 0, "CityNode::connect", "other node is no CityNode");

		CityNode * c = static_cast<CityNode *>(n);
		score += c->score;
		open += c->open - 2;
		Node::connect(n);
	}
};

class Tile
{
	friend class TileFactory;

public:
	enum Side { left = 0, up = 1, right = 2, down = 3 };
	enum TileSet { BaseGame = 0 };
	Q_DECLARE_FLAGS(TileSets, TileSet)
#if NODE_VARIANT
	typedef Node ** EdgeType;
#else
	typedef Node * EdgeType;
#endif

	TerrainType const * edges; // array of edge types
	int const nodeCount;       // lenght of nodes
	Side orientation = left;

private:
	Node ** const nodes;       // array of node pointers
	EdgeType * edgeNodes[4];   // 4 arrays of edge connectors

public:
	//Not neccessarily the best place (because this will need to be copied also):
	TileSet const tileSet;
	int tileType;

public:
	Tile(TileSet tileSet, int tileType, TerrainType const edges[4], const int nodeCount, Node ** nodes);
//	Tile(const Tile & t);
	~Tile();
	TerrainType const & getEdge(Side side) const;
	TerrainType const & getEdge(Side side, Side orientation) const;
	bool connect(Side s, Tile * other);

private:
	EdgeType * getEdgeNodes(Side side);
	void setEdgeNode(Side side, int index, Node *& n);

	static EdgeType * createEdgeList(TerrainType t);
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
};
Q_DECLARE_OPERATORS_FOR_FLAGS(Tile::TileSets)

class TileFactory
{
public:
	static QList<Tile *> createTiles(Tile::TileSets s);

//private:
	static Tile * createTile(Tile::TileSet s, int type);
};

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
