#include "tile.h"

#include "game.h"

#include <unordered_map>

#include <QDebug>

Node::Node(TerrainType t, Tile * parent, const Game * g)
	: t(t),
	  meeples(new uchar[g->getPlayerCount()]())
{
	Q_ASSERT(g != 0);
	tiles.insert(parent);
}

Node::~Node()
{
	delete meeples;
}

void Node::connect(Node * n, Game * g)
{
	if (this == n)
		return;

	for (Node ** p : n->pointers)
		*p = this;
	pointers.append(n->pointers);
	tiles.insert(n->tiles.begin(), n->tiles.end());
	for (uchar * tm = meeples, * end = meeples + (g->getPlayerCount()), * nm = n->meeples;
		 tm < end;
		 ++tm, ++nm)
	{
		*tm += *nm;
	}

	delete n;
}

void CityNode::connect(Node * n, Game * g)
{
	Q_ASSERT_X(n->t == this->t, "CityNode::connect", "TerrainType does not match");
	Q_ASSERT_X(typeid(*n) == typeid(*this), "CityNode::connect", "other node is no CityNode");

	open -= 2;
	if (this != n)
	{
		CityNode * c = static_cast<CityNode *>(n);
		bonus += c->bonus;
		open += c->open;
		Node::connect(n, g);
	}
	Q_ASSERT(uchar(open + 2) > open);

	qDebug() << "   city open:" << open;
	if (open == 0)
		g->cityClosed(this);
}

void RoadNode::connect(Node * n, Game * g)
{
	Q_ASSERT_X(n->t == this->t, "RoadNode::connect", "TerrainType does not match");
	Q_ASSERT_X(typeid(*n) == typeid(*this), "RoadNode::connect", "other node is no RoadNode");

	open -= 2;
	if (this != n)
	{
		RoadNode * r = static_cast<RoadNode *>(n);
		open += r->open;
		Node::connect(n, g);
	}
	Q_ASSERT(uchar(open + 2) > open);

	qDebug() << "   road open:" << open;

	if (open == 0)
		g->roadClosed(this);
}

Tile::Tile(TileSet tileSet, int tileType)
	: edges {None, None, None, None},
	  tileSet(tileSet),
	  tileType(tileType)
{
}

Tile::Tile(TileSet tileSet, int tileType, TerrainType const edges[4], int const nodeCount, Node ** nodes)
	: edges { edges[0], edges[1], edges[2], edges[3] },
	  nodeCount(nodeCount),
	  nodes(nodes),
	  tileSet(tileSet),
	  tileType(tileType)
{
	createEdgeList(left);
	createEdgeList(up);
	createEdgeList(right);
	createEdgeList(down);

	for (int i = 0; i < nodeCount; ++i)
		nodes[i]->pointers.append(nodes + i);
}

Tile::~Tile()
{
	delete[] nodes;
	delete[] edgeNodes[left];
	delete[] edgeNodes[up];
	delete[] edgeNodes[right];
	delete[] edgeNodes[down];
}

const TerrainType &Tile::getEdge(Tile::Side side) const
{
	return edges[(4 + side - orientation) % 4];
}

const TerrainType & Tile::getEdge(Side side, Side orientation) const
{
	return edges[(4 + side - orientation) % 4];
}

bool Tile::connect(Tile::Side side, Tile * other, Game * game)
{
	Tile::Side otherSide = (Tile::Side)((side + 2) % 4);
	TerrainType t = getEdge(side);

	Q_ASSERT_X(t == other->getEdge(otherSide), "Tile::connect", "edges don't match");
//	if (t != other->getEdge(otherSide))
//		return false;

	EdgeType * nodeList = getEdgeNodes(side);
	EdgeType * otherNodeList = other->getEdgeNodes(otherSide);
	int const nodeCount = edgeNodeCount(t);
	for (int i = 0; i < nodeCount; ++i)
	{
		EdgeType otherNode = otherNodeList[i];
		EdgeType thisNode = nodeList[nodeCount - i - 1];

#if NODE_VARIANT
		(*thisNode)->connect(*otherNode, game);
#else
		thisNode->connect(otherNode, game);
#endif
	}

	return true;
}

Tile * Tile::clone(const Game * g)
{
	Tile * copy = new Tile(tileSet, tileType, edges, 0, 0);
	copy->orientation = orientation;

	copy->nodeCount = nodeCount;
	copy->nodes = new Node*[nodeCount];

	std::unordered_map<EdgeType, EdgeType> nodeMap;
	for (int i = 0; i < nodeCount; ++i)
	{
		copy->nodes[i] = nodes[i]->clone(copy, g);
		copy->nodes[i]->pointers.append(copy->nodes + i);
#if NODE_VARIANT
		nodeMap[nodes + i] = copy->nodes + i;
#else
		nodeMap[nodes[i]] = copy->nodes[i];
#endif
	}
	for (int i = 0; i < 4; ++i)
	{
		int const enc = edgeNodeCount(edges[i]);
		for (int j = 0; j < enc; ++j)
#if NODE_VARIANT
			copy->edgeNodes[(Side)i][j] = nodeMap[edgeNodes[(Side)i][j]];
#else
			copy->setEdgeNode((Side)i, j, nodeMap[edgeNodes[(Side)i][j]]);
#endif
	}
	return copy;
}

Tile::EdgeType * Tile::getEdgeNodes(Tile::Side side)
{
	return edgeNodes[(4 + side - orientation) % 4];
}

void Tile::setEdgeNode(Tile::Side side, int index, Node * n)
{
#if NODE_VARIANT
	for (int i = 0; i < nodeCount; ++i)
	{
		if (nodes[i] == n)
		{
			edgeNodes[side][index] = nodes + i;
			break;
		}
	}
#else
	edgeNodes[side][index] = n;
	n->pointers.append(&(edgeNodes[side][index]));
#endif
}

void Tile::createEdgeList(Side side)
{
	edgeNodes[side] = new EdgeType[edgeNodeCount(edges[side])];
}

void Tile::printSides(Node * n)
{
	for (int i = 0; i < 4; ++i)
		for (int j = 0; j < edgeNodeCount(getEdge((Side)i)); ++j)
			if (getEdgeNodes((Side)i)[j] == n)
				qDebug() << "\t" << (Side)i << j;
}
