#include "tile.h"

#include "game.h"

#include <unordered_map>

#define PRINT_CONNECTIONS 0


#if PRINT_CONNECTIONS
#include <QDebug>
#endif

Node::Node(TerrainType t, const Tile * parent, const Game * g)
    : data(this, g->getPlayerCount())
{
	Q_ASSERT(g != 0);
	
	data.t = t;
	
	data.tiles.insert(parent);
}

Node::~Node()
{
//	for (Node ** p : pointers)
//	{
//		if (p)
//			*p = 0;
//	}
}

void Node::removeMeeple(int player, Game * g)
{
	checkUnclose(g);
	if (d->meeples[player]-- == d->maxMeples)
	{
		d->maxMeples = 0;
		for (int i = 0; i < g->getPlayerCount(); ++i)
			if (d->meeples[i] > d->maxMeples)
				d->maxMeples = d->meeples[i];
	}
}

void Node::connect(Node * n, Game * g)
{
	Q_ASSERT_X(n->getTerrain() == this->getTerrain(), "Node::connect", "TerrainType does not match");
	Q_ASSERT_X(typeid(*n) == typeid(*this), "Node::connect", "classes do not match");
	Q_ASSERT(getScored() == NotScored);
	Q_ASSERT(n->getScored() == NotScored);
	Q_ASSERT(data.scored == NotScored);
	Q_ASSERT(n->data.scored == NotScored);

#if PRINT_CONNECTIONS
	qDebug() << "  connect:" << n->id() << "->" << id() << "   BEFORE";
//	for (Tile const * t : d->tiles)
//		qDebug() << "    " << id() << t->id;
//	for (Tile const * t : n->d->tiles)
//		qDebug() << "    " << n->id() << t->id;
#endif
	
	if (!isSame(n))
	{
//	for (Node ** & p : n->pointers)
//	{
//		*p = this;
//		pointers.push_back(p);
////		p = 0;
//	}
		d->tiles.insert(n->d->tiles.begin(), n->d->tiles.end());
		for (uchar * tm = d->meeples, * end = d->meeples + (g->getPlayerCount()), * nm = n->d->meeples;
			 tm < end;
			 ++tm, ++nm)
		{
			*tm += *nm;
			if (*tm > d->maxMeples)
				d->maxMeples = *tm;
		}
		
		if (isOccupied())
			checkClose(g);
		
		d->nodes.insert(n->d->nodes.begin(), n->d->nodes.end());
#if PRINT_CONNECTIONS
//		qDebug() << "  connect:" << n->id() << "->" << id();
#endif
	}
#if PRINT_CONNECTIONS
	else
	{
//		qDebug() << "  connect:" << n->id() << "->" << id() << "skipped";
		qDebug() << "  skpped";
	}
	
//	qDebug() << "  connect:" << n->id() << "->" << id() << "   AFTER";
//	for (Tile const * t : d->tiles)
//		qDebug() << "    " << id() << t->id;
//	for (Tile const * t : n->d->tiles)
//		qDebug() << "    " << n->id() << t->id;
#endif
	
//	Q_ASSERT(n->d == &n->data);

	for (Node * o : d->nodes)
	{
		if (o == this)
			continue;
		o->ds.push_back(d);
		o->d = d;
	}
}

void Node::disconnect(Node * n, Game * g)	//only works in reverse order of connecting.
{
	for (Node * o : d->nodes)
	{
		if (o == this)
			continue;
		o->ds.pop_back();
		Q_ASSERT(o->ds.size() > 0);
		o->d = o->ds.back();
	}
	
#if PRINT_CONNECTIONS
	qDebug() << "  disconnect:" << n->id() << "->" << id() << "   BEFORE";
//	for (Tile const * t : d->tiles)
//		qDebug() << "    " << id() << t->id;
//	for (Tile const * t : n->d->tiles)
//		qDebug() << "    " << n->id() << t->id;
#endif
	
	if (!isSame(n))
	{	
		for (auto const & t : n->d->nodes)
		{
			auto const & i = d->nodes.find(t);
			Q_ASSERT(i != d->nodes.end());
			d->nodes.erase(i);
		}
		
		if (isOccupied())
			checkUnclose(g);
		
		d->maxMeples = 0;
		for (uchar * tm = d->meeples, * end = d->meeples + (g->getPlayerCount()), * nm = n->d->meeples;
			 tm < end;
			 ++tm, ++nm)
		{
			*tm -= *nm;
			if (*tm > d->maxMeples)
				d->maxMeples = *tm;
		}
		
		for (auto const & t : n->d->tiles)
			d->tiles.erase(d->tiles.find(t));
		
	//	for (Node ** & p : n->pointers)
	//	{
	//		*p = n;
	//		pointers.pop_back();
	////		p = ?;
	//	}
		
#if PRINT_CONNECTIONS
	//	qDebug() << "  disconnect:" << n->id() << "->" << id();
#endif
	}
#if PRINT_CONNECTIONS
	else
	{
//		qDebug() << "  disconnect:" << n->id() << "->" << id() << "skipped";
		qDebug() << "  skipped";
		return;
	}
	
//	qDebug() << "  disconnect:" << n->id() << "->" << id() << "   AFTER";
//	for (Tile const * t : d->tiles)
//		qDebug() << "    " << id() << t->id;
//	for (Tile const * t : n->d->tiles)
//		qDebug() << "    " << n->id() << t->id;
#endif
}

bool Node::equals(const Node & other, const Game * g) const
{
	if (ds.size() != other.ds.size())
		return false;
	if (data.t != other.data.t)
		return false;
	if (data.tiles.size() != other.data.tiles.size())
		return false;
	for (int i = 0; i < g->getPlayerCount(); ++i)
		if (data.meeples[i] != other.data.meeples[i])
			return false;
	if (data.maxMeples != other.data.maxMeples)
		return false;
	if (data.scored != other.data.scored)
		return false;
	if (data.nodes.size() != other.data.nodes.size())
		return false;
#if DEBUG_IDS
	if (data.id != other.data.id)
		return false;
#endif
	
	if ((d == &data) != (other.d == &other.data))
		return false;
	if (d != &data)
	{
		if (d->t != other.d->t)
			return false;
		if (d->tiles.size() != other.d->tiles.size())
			return false;
		for (int i = 0; i < g->getPlayerCount(); ++i)
			if (d->meeples[i] != other.d->meeples[i])
				return false;
		if (d->maxMeples != other.d->maxMeples)
			return false;
		if (d->scored != other.d->scored)
			return false;
		if (d->nodes.size() != other.d->nodes.size())
			return false;
#if DEBUG_IDS
		if (d->id != other.d->id)
			return false;
#endif
	}
	
	return true;
}

void FieldNode::connect(Node * n, Game * g)
{
	if (!isSame(n))
	{
		FieldNode * f = static_cast<FieldNode *>(n);
		getFieldData()->cities.insert(getFieldData()->cities.end(), f->getFieldData()->cities.begin(), f->getFieldData()->cities.end());
	}
	
	Node::connect(n, g);
}

void FieldNode::disconnect(Node * n, Game * g)
{
	Node::disconnect(n, g);
	if (!isSame(n))
	{
		FieldNode * f = static_cast<FieldNode *>(n);
		getFieldData()->cities.erase(getFieldData()->cities.end() - f->getFieldData()->cities.size(), getFieldData()->cities.end());
	}
}

void CityNode::connect(Node * n, Game * g)
{
	getCityData()->open -= 2;
	if (!isSame(n))
	{
		CityNode * c = static_cast<CityNode *>(n);
		getCityData()->bonus += c->getCityData()->bonus;
		getCityData()->open += c->getCityData()->open;
	}
	Node::connect(n, g);
	Q_ASSERT(uchar(getCityData()->open + 2) > getCityData()->open);
}

void CityNode::disconnect(Node * n, Game * g)
{
	Node::disconnect(n, g);
	if (!isSame(n))
	{
		CityNode * c = static_cast<CityNode *>(n);
		getCityData()->open -= c->getCityData()->open;
		getCityData()->bonus -= c->getCityData()->bonus;
	}
	getCityData()->open += 2;
}

void CityNode::checkClose(Game * g)
{
	if (getCityData()->open == 0)
		g->cityClosed(this);
}

void CityNode::checkUnclose(Game * g)
{
	if (getCityData()->open == 0)
		g->cityUnclosed(this);
}

void RoadNode::connect(Node * n, Game * g)
{
	Q_ASSERT_X(n->getTerrain() == this->getTerrain(), "RoadNode::connect", "TerrainType does not match");
	Q_ASSERT_X(typeid(*n) == typeid(*this), "RoadNode::connect", "other node is no RoadNode");
	
	getRoadData()->open -= 2;
	if (!isSame(n))
	{
		RoadNode * r = static_cast<RoadNode *>(n);
		getRoadData()->open += r->getRoadData()->open;
	}
	Node::connect(n, g);
	Q_ASSERT(uchar(getRoadData()->open + 2) > getRoadData()->open);
}

void RoadNode::disconnect(Node * n, Game * g)
{
	Node::disconnect(n, g);
	if (!isSame(n))
	{
		RoadNode * r = static_cast<RoadNode *>(n);
		getRoadData()->open -= r->getRoadData()->open;
	}
	getRoadData()->open += 2;
}

void RoadNode::checkClose(Game * g)
{
	if (getRoadData()->open == 0)
		g->roadClosed(this);
}

void RoadNode::checkUnclose(Game * g)
{
	if (getRoadData()->open == 0)
		g->roadUnclosed(this);
}

void CloisterNode::checkClose(Game *g)
{
	if (surroundingTiles == 9)
		g->cloisterClosed(this);
}

void CloisterNode::checkUnclose(Game * g)
{
	if (surroundingTiles == 9)
		g->cloisterUnclosed(this);
}



Tile::Tile(TileSet tileSet, uchar tileType)
	: edges {None, None, None, None},
	  tileSet(tileSet),
	  tileType(tileType)
{
}

Tile::Tile(TileSet tileSet, uchar tileType, TerrainType const edges[4])
	: edges { edges[0], edges[1], edges[2], edges[3] },
	  tileSet(tileSet),
	  tileType(tileType)
{
	createEdgeList(left);
	createEdgeList(up);
	createEdgeList(right);
	createEdgeList(down);
}

Tile::~Tile()
{
	for (int i = 0; i < nodeCount; ++i)
		delete nodes[i];
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

void Tile::connect(Tile::Side side, Tile * other, Game * game)
{
#if PRINT_CONNECTIONS
	qDebug() << "connect tile:" << id << "<->" << other->id;
#endif
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
	
	if (cloister != 0)
		cloister->addSurroundingTile(game);
	if (other->cloister != 0)
		other->cloister->addSurroundingTile(game);
}

void Tile::disconnect(Tile::Side side, Tile * other, Game * game)
{
#if PRINT_CONNECTIONS
	qDebug() << "disconnect tile:" << id << "<->" << other->id;
#endif
	Tile::Side otherSide = (Tile::Side)((side + 2) % 4);
	TerrainType t = getEdge(side);

	Q_ASSERT_X(t == other->getEdge(otherSide), "Tile::connect", "edges don't match");
	
	if (other->cloister != 0)
		other->cloister->removeSurroundingTile(game);
	if (cloister != 0)
		cloister->removeSurroundingTile(game);
	
	int const nodeCount = edgeNodeCount(t);
	EdgeType * nodeList = getEdgeNodes(side);
	EdgeType * otherNodeList = other->getEdgeNodes(otherSide);
	for (int i = 0; i < nodeCount; ++i)
	{
		EdgeType otherNode = otherNodeList[nodeCount - i - 1];
		EdgeType thisNode = nodeList[i];
//		EdgeType otherNode = otherNodeList[i];
//		EdgeType thisNode = nodeList[nodeCount - i - 1];
		
#if NODE_VARIANT
		(*thisNode)->disconnect(*otherNode, game);
#else
		thisNode->disconnect(otherNode, game);
#endif
	}
}

void Tile::connectDiagonal(Tile * other, Game * game)
{
	if (cloister != 0)
		cloister->addSurroundingTile(game);
	if (other->cloister != 0)
		other->cloister->addSurroundingTile(game);
}

void Tile::disconnectDiagonal(Tile * other, Game * game)
{
	if (other->cloister != 0)
		other->cloister->removeSurroundingTile(game);
	if (cloister != 0)
		cloister->removeSurroundingTile(game);
}

Tile * Tile::clone(const Game * g)	//TODO? This process only works on unconnected tiles.
{
	Tile * copy = new Tile(tileSet, tileType, edges);
	copy->orientation = orientation;

	copy->nodeCount = nodeCount;
	copy->nodes = new Node*[nodeCount];

	std::unordered_map<EdgeType, EdgeType> nodeMap;
	for (int i = 0; i < nodeCount; ++i)
	{
		copy->nodes[i] = nodes[i]->clone(copy, g);
//		copy->nodes[i]->pointers.push_back(copy->nodes + i);
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
	
	for (int i = 0; i < nodeCount; ++i)
	{
		switch (nodes[i]->getTerrain())
		{
			case Field:
			{
				FieldNode * tf = static_cast<FieldNode *>(nodes[i]);
				FieldNode * cf = static_cast<FieldNode *>(copy->nodes[i]);
				for (CityNode * c : tf->getFieldData()->cities)
					cf->getFieldData()->cities.push_back(static_cast<CityNode *>(nodeMap[c]));
				break;
			}
			default:
				break;
		}
	}
	
	if (cloister)
		copy->cloister = static_cast<CloisterNode *>(nodeMap[cloister]);
	
	return copy;
}

Tile::EdgeType * Tile::getEdgeNodes(Tile::Side side)
{
	return edgeNodes[(4 + side - orientation) % 4];
}

void Tile::setEdgeNode(Tile::Side side, uchar index, Node * n)
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
//	n->pointers.push_back(&(edgeNodes[side][index]));
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

bool Tile::equals(const Tile & other, Game const * g) const
{
	if (!(tileType == other.tileType) && (tileSet == other.tileSet) && (orientation == other.orientation))
		return false;
	if (nodeCount != other.nodeCount)
		return false;
	for (int i = 0; i < nodeCount; ++i)
		if (!nodes[i]->equals(*other.nodes[i], g))
			return false;
	return true;
}
