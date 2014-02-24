#include "tile.h"

#include <unordered_map>

#include <QDebug>

//int Node::nextId = 0;

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

Tile::Tile(const Tile & t)
	: Tile(t.tileSet, t.tileType, t.edges, 0, 0)
{
	orientation = t.orientation;

	nodeCount = t.nodeCount;
	nodes = new Node*[t.nodeCount];
	std::unordered_map<EdgeType, EdgeType> nodeMap;
	for (int i = 0; i < nodeCount; ++i)
	{
		nodes[i] = t.nodes[i]->clone();
		nodes[i]->pointers.append(nodes + i);
#if NODE_VARIANT
		nodeMap[t.nodes + i] = nodes + i;
#else
		nodeMap[t.nodes[i]] = nodes[i];
#endif
	}
	for (int i = 0; i < 4; ++i)
	{
		int const enc = edgeNodeCount(edges[i]);
		for (int j = 0; j < enc; ++j)
#if NODE_VARIANT
			edgeNodes[(Side)i][j] = nodeMap[t.edgeNodes[(Side)i][j]];
#else
			setEdgeNode((Side)i, j, nodeMap[t.edgeNodes[(Side)i][j]]);
#endif
	}
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

bool Tile::connect(Tile::Side side, Tile * other)
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
		(*thisNode)->connect(*otherNode);
#else
		thisNode->connect(otherNode);
#endif
	}

	return true;
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
