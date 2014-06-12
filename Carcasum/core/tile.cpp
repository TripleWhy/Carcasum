#include "tile.h"
#include "game.h"
#include <unordered_map>
#include <typeinfo>

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
		for (uint i = 0; i < g->getPlayerCount(); ++i)
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
			*tm = (uchar)(*tm + *nm);
			if (*tm > d->maxMeples)
				d->maxMeples = *tm;
		}
		
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

	if (isOccupied())
		checkClose(g);
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

	if (isOccupied())
		checkUnclose(g);
	
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
		
		d->maxMeples = 0;
		for (uchar * tm = d->meeples, * end = d->meeples + (g->getPlayerCount()), * nm = n->d->meeples;
			 tm < end;
			 ++tm, ++nm)
		{
			*tm = (uchar)(*tm - *nm);
			if (*tm > d->maxMeples)
				d->maxMeples = *tm;
		}
		
		for (auto const & t : n->d->tiles)
			d->tiles.erase(d->tiles.find(t));
		
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

#if USE_RESET
void Node::reset(Tile const * parent, Game const * g)
{
	data.tiles.clear();
	data.tiles.insert(parent);
	for (uchar * m = data.meeples, * end = data.meeples + g->getPlayerCount(); m < end; ++m)
		*m = 0;
	data.maxMeples = 0;
	data.scored = NotScored;
	data.nodes.clear();
	data.nodes.insert(this);
	d = &data;
	ds.clear();
	ds.push_back(d);
}
#endif

bool Node::equals(const Node & other, const Game * g) const
{
	if (ds.size() != other.ds.size())
		return false;
	if (data.t != other.data.t)
		return false;
	if (data.tiles.size() != other.data.tiles.size())
		return false;
	for (uint i = 0; i < g->getPlayerCount(); ++i)
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
		for (uint i = 0; i < g->getPlayerCount(); ++i)
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

#if USE_RESET
void FieldNode::reset(const Tile * parent, const Game * g)
{
	Node::reset(parent, g);
	fieldData = originalFieldData;
}
#endif

void CityNode::connect(Node * n, Game * g)
{
	getCityData()->open = (uchar)(getCityData()->open - 2);
	if (!isSame(n))
	{
		CityNode * c = static_cast<CityNode *>(n);
		getCityData()->bonus = (uchar)(getCityData()->bonus + c->getCityData()->bonus);
		getCityData()->open = (uchar)(getCityData()->open + c->getCityData()->open);
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
		getCityData()->open = (uchar)(getCityData()->open - c->getCityData()->open);
		getCityData()->bonus = (uchar)(getCityData()->bonus - c->getCityData()->bonus);
	}
	getCityData()->open = (uchar)(getCityData()->open + 2);
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

#if USE_RESET
void CityNode::reset(const Tile * parent, const Game * g)
{
	Node::reset(parent, g);
	cityData = originalCityData;
}
#endif

void RoadNode::connect(Node * n, Game * g)
{
	Q_ASSERT_X(n->getTerrain() == this->getTerrain(), "RoadNode::connect", "TerrainType does not match");
	Q_ASSERT_X(typeid(*n) == typeid(*this), "RoadNode::connect", "other node is no RoadNode");
	
	getRoadData()->open = (uchar)(getRoadData()->open - 2);
	if (!isSame(n))
	{
		RoadNode * r = static_cast<RoadNode *>(n);
		getRoadData()->open = (uchar)(getRoadData()->open + r->getRoadData()->open);
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
		getRoadData()->open = (uchar)(getRoadData()->open - r->getRoadData()->open);
	}
	getRoadData()->open = (uchar)(getRoadData()->open + 2);
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

#if USE_RESET
void RoadNode::reset(const Tile * parent, const Game * g)
{
	Node::reset(parent, g);
	roadData = originalRoadData;
}
#endif

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

#if USE_RESET
void CloisterNode::reset(const Tile * parent, const Game * g)
{
	Node::reset(parent, g);
	surroundingTiles = 1;
}
#endif


Tile::~Tile()
{
	for (int i = 0; i < nodeCount; ++i)
		delete nodes[i];
	delete[] nodes;
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
	Tile::Side otherSide = sideOpposite(side);

	Q_ASSERT_X(getEdge(side) == other->getEdge(otherSide), "Tile::connect", "edges don't match");
//	if (t != other->getEdge(otherSide))
//		return false;

	EdgeType * nodeList = getEdgeNodes(side);
	EdgeType * otherNodeList = other->getEdgeNodes(otherSide);
	for (int i = 0; i < EDGE_NODE_COUNT; ++i)
	{
		EdgeType thisNode = nodeList[EDGE_NODE_COUNT - 1 - i];
		if (isNull(thisNode))
			continue;
		EdgeType otherNode = otherNodeList[i];

#if NODE_VARIANT == 1
		(*thisNode)->connect(*otherNode, game);
#elif NODE_VARIANT == 0
		thisNode->connect(otherNode, game);
#elif NODE_VARIANT == 2
		nodes[thisNode]->connect(other->nodes[otherNode], game);
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
	Tile::Side otherSide = sideOpposite(side);

	Q_ASSERT_X(getEdge(side) == other->getEdge(otherSide), "Tile::connect", "edges don't match");
	
	if (other->cloister != 0)
		other->cloister->removeSurroundingTile(game);
	if (cloister != 0)
		cloister->removeSurroundingTile(game);

	EdgeType * nodeList = getEdgeNodes(side);
	EdgeType * otherNodeList = other->getEdgeNodes(otherSide);
	for (int i = 0; i < EDGE_NODE_COUNT; ++i)
	{
		EdgeType thisNode = nodeList[i];
		if (isNull(thisNode))
			continue;
		EdgeType otherNode = otherNodeList[EDGE_NODE_COUNT - 1 - i];

#if NODE_VARIANT == 1
		(*thisNode)->disconnect(*otherNode, game);
#elif NODE_VARIANT == 0
		thisNode->disconnect(otherNode, game);
#elif NODE_VARIANT == 2
		nodes[thisNode]->disconnect(other->nodes[otherNode], game);
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
	Tile * copy = new Tile(tileType, edges);
	copy->orientation = orientation;

	copy->nodeCount = nodeCount;
	copy->nodes = new Node*[nodeCount];

	std::unordered_map<Node *, Node *> nodeMap;
	for (int i = 0; i < nodeCount; ++i)
	{
		copy->nodes[i] = nodes[i]->clone(copy, g);
//		copy->nodes[i]->pointers.push_back(copy->nodes + i);
		nodeMap[nodes[i]] = copy->nodes[i];
	}
	
	for (int i = 0; i < 4; ++i)
	{
		for (int j = 0; j < EDGE_NODE_COUNT; ++j)
#if NODE_VARIANT == 1
			if (edgeNodes[i][j] != 0)
				copy->edgeNodes[i][j] = copy->nodes + ( edgeNodes[i][j] - nodes );
#elif NODE_VARIANT == 0
			copy->setEdgeNode((Side)i, j, nodeMap[edgeNodes[(Side)i][j]]);
#elif NODE_VARIANT == 2
			copy->edgeNodes[i][j] = edgeNodes[i][j];
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

#if USE_RESET
void Tile::reset(Game const * g)
{
	for (Node ** n = nodes, ** end = nodes + nodeCount; n < end; ++n)
		(*n)->reset(this, g);
	orientation = left;
}
#endif

Tile::EdgeType * Tile::getEdgeNodes(Tile::Side side)
{
	return edgeNodes[(4 + side - orientation) % 4];
}

Tile::EdgeType *Tile::getEdgeNodes(Tile::Side side, Tile::Side orientation)
{
	return edgeNodes[(4 + side - orientation) % 4];
}

const Tile::EdgeType *Tile::getEdgeNodes(Tile::Side side) const
{
	return edgeNodes[(4 + side - orientation) % 4];
}

const Tile::EdgeType *Tile::getEdgeNodes(Tile::Side side, Tile::Side orientation) const
{
	return edgeNodes[(4 + side - orientation) % 4];
}

void Tile::setEdgeNode(Tile::Side side, int index, Node * n)
{
#if NODE_VARIANT == 1
	if (n == 0)
		edgeNodes[side][index] = 0;
	else
	{
		for (int i = 0; i < nodeCount; ++i)
		{
			if (nodes[i] == n)
			{
				edgeNodes[side][index] = nodes + i;
				break;
			}
		}
	}
#elif NODE_VARIANT == 0
	edgeNodes[side][index] = n;
//	n->pointers.push_back(&(edgeNodes[side][index]));
#elif NODE_VARIANT == 2
	if (n == 0)
		edgeNodes[side][index] = -1;
	else
	{
		for (uchar i = 0; i < nodeCount; ++i)
		{
			if (nodes[i] == n)
			{
				edgeNodes[side][index] = i;
				break;
			}
		}
		Q_ASSERT(edgeNodes[side][index] != -1);
	}
#endif
}

//void Tile::printSides(Node * n)
//{
//	for (int i = 0; i < 4; ++i)
//		for (int j = 0; j < EDGE_NODE_COUNT; ++j)
//			if (getEdgeNodes((Side)i)[j] == n)
//				qDebug() << "\t" << (Side)i << j;
//}

bool Tile::equals(const Tile & other, Game const * g) const
{
	if (!(tileType == other.tileType) && (orientation == other.orientation))
		return false;
	if (nodeCount != other.nodeCount)
		return false;
	for (int i = 0; i < nodeCount; ++i)
		if (!nodes[i]->equals(*other.nodes[i], g))
			return false;
	return true;
}
