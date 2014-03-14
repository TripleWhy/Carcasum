#include "tile.h"

#include "game.h"

#include <unordered_map>

#include <QDebug>

Node::Node(TerrainType t, const Tile * parent, const Game * g)
	: t(t),
	  meeples(new uchar[g->getPlayerCount()]())
{
	Q_ASSERT(g != 0);
	
	tiles.insert(parent);
}

Node::~Node()
{
	delete[] meeples;
	p = 0;
}

void Node::removeMeeple(int player, Game * g)
{
	if (p)
		return p->removeMeeple(player, g);
	checkUnclose(g);
	if (meeples[player]-- == maxMeples)
	{
		maxMeples = 0;
		for (int i = 0; i < g->getPlayerCount(); ++i)
			if (meeples[i] > maxMeples)
				maxMeples = meeples[i];
	}
}

bool Node::connect2(Node * n, Game * g)
{
//	if (p)
//		return p->connect(n, g);
	
	Q_ASSERT_X(n->t == this->t, "Node::connect", "TerrainType does not match");
	Q_ASSERT_X(typeid(*n) == typeid(*this), "Node::connect", "classes do not match");
	
	Node * thisTarget = getTarget();
	n = n->getTargetButNot(this);
	if (n->p == this)
	{
		++connections;
		return true;
	}
	n = n->getTarget();
	if (n == thisTarget)
		return true;

	thisTarget->tiles.insert(n->tiles.begin(), n->tiles.end());
	for (uchar * tm = thisTarget->meeples, * end = thisTarget->meeples + (g->getPlayerCount()), * nm = n->meeples;
		 tm < end;
		 ++tm, ++nm)
	{
		*tm += *nm;
		if (*tm > thisTarget->maxMeples)
			thisTarget->maxMeples = *tm;
	}
	
	if (thisTarget->isOccupied())
		thisTarget->checkClose(g);
	
	n->p = this;
	Q_ASSERT(thisTarget->connections == 0);
	thisTarget->connections = 1;
	qDebug() << "  connect:" << n->id << "->" << id;
	return false;
}

bool Node::disconnect2(Node * n, Game * g)	//only works in reverse order of connecting.
{
//	if (p)
//		return p->disconnect(n, g);
	
	Q_ASSERT(n != 0);
	Q_ASSERT(g != 0);
	
//	Node * thisTarget = getTarget();
//	n = n->getTargetButNot(this);
//	if (n == thisTarget)
//		return;
//	Q_ASSERT(n->p == this);
	Node * thisTarget = getTarget();
	n = n->getTargetButNot(this);
	if (n->p == this && thisTarget->connections > 1)
	{
		--connections;
		return true;
	}
	n = n->getTarget();
	if (n == thisTarget)
		return true;
	
	if (thisTarget->isOccupied())
		thisTarget->checkUnclose(g);
	
	for (auto const & t : n->tiles)
		thisTarget->tiles.erase(t);
	
	thisTarget->maxMeples = 0;
	for (uchar * tm = thisTarget->meeples, * end = thisTarget->meeples + (g->getPlayerCount()), * nm = n->meeples;
		 tm < end;
		 ++tm, ++nm)
	{
		*tm -= *nm;
		if (*tm > thisTarget->maxMeples)
			thisTarget->maxMeples = *tm;
	}
	
	n->p = 0;
	Q_ASSERT(thisTarget->connections == 1);
	thisTarget->connections = 0;
	qDebug() << "  disconnect:" << n->id << "->" << id;
	
	return false;
}

bool Node::equals(const Node & other, const Game * g) const
{
	if (t != other.t)
		return false;
	if (tiles.size() != other.tiles.size())
		return false;
	for (int i = 0; i < g->getPlayerCount(); ++i)
		if (meeples[i] != other.meeples[i])
			return false;
	if (maxMeples != other.maxMeples)
		return false;
	if ((p == 0) != (other.p == 0))
		return false;
	return true;
}

void FieldNode::connect(Node * n, Game * g)
{
//	if (p)
//		return p->connect(n, g);
	FieldNode * thisTarget = static_cast<FieldNode *>(getTarget());
	
	n = n->getTarget();
	if (n == thisTarget)
		return;
Q_ASSERT(n != this);
	
	FieldNode * f = static_cast<FieldNode *>(n);
	thisTarget->cities.insert(thisTarget->cities.end(), f->cities.begin(), f->cities.end());
	
	Node::connect(n, g);
}

void FieldNode::disconnect(Node * n, Game * g)
{
//	if (p)
//		return p->disconnect(n, g);
	FieldNode * thisTarget = static_cast<FieldNode *>(getTarget());
	
	n = n->getTargetButNot(this);
Q_ASSERT(n != this);
	if (n == thisTarget)
		return;
	
	Node::disconnect(n, g);
	
	FieldNode * f = static_cast<FieldNode *>(n);
	thisTarget->cities.erase(thisTarget->cities.end() - f->cities.size(), cities.end());
}

void CityNode::connect(Node * n, Game * g)
{
//	if (p)
//		return p->connect(n, g);
	CityNode * thisTarget = static_cast<CityNode *>(getTarget());
	
	thisTarget->open -= 2;
	n = n->getTarget();
Q_ASSERT(n != this);
	if (n != thisTarget)
	{
		CityNode * c = static_cast<CityNode *>(n);
		thisTarget->bonus += c->bonus;
		thisTarget->open += c->open;
		
		Node::connect(n, g);
	}
	Q_ASSERT(uchar(thisTarget->open + 2) > thisTarget->open);
}

void CityNode::disconnect(Node * n, Game * g)
{
//	if (p)
//		return p->disconnect(n, g);
	CityNode * thisTarget = static_cast<CityNode *>(getTarget());
	
	n = n->getTargetButNot(this);
	if (n != thisTarget)
	{
		Node::disconnect(n, g);
		
		CityNode * c = static_cast<CityNode *>(n);
		thisTarget->bonus -= c->bonus;
		thisTarget->open -= c->open;
	}
	thisTarget->open += 2;
}

void CityNode::checkClose(Game * g)
{
	if (p)
		return p->checkClose(g);
	
	if (open == 0)
		g->cityClosed(this);
}

void CityNode::checkUnclose(Game * g)
{
	if (p)
		return p->checkUnclose(g);
	
	if (open == 0)
		g->cityUnclosed(this);
}

void RoadNode::connect(Node * n, Game * g)
{
	if (p)
		return p->connect(n, g);
	
	Q_ASSERT_X(n->t == this->t, "RoadNode::connect", "TerrainType does not match");
	Q_ASSERT_X(typeid(*n) == typeid(*this), "RoadNode::connect", "other node is no RoadNode");
	
	open -= 2;
	n = n->getTarget();
	if (n != this)
	{
		RoadNode * r = static_cast<RoadNode *>(n);
		open += r->open;
		Node::connect(n, g);
	}
	Q_ASSERT(uchar(open + 2) > open);
}

void RoadNode::disconnect(Node * n, Game * g)
{
	if (p)
		return p->disconnect(n, g);
	
	n = n->getTargetButNot(this);
	if (n != this)
	{
		RoadNode * r = static_cast<RoadNode *>(n);
		Node::disconnect(n, g);
		open -= r->open;
	}
	open += 2;
//	Q_ASSERT(uchar(open - 2) < open);
}

void RoadNode::checkClose(Game * g)
{
	if (p)
		return p->checkClose(g);
	
	if (open == 0)
		g->roadClosed(this);
}

void RoadNode::checkUnclose(Game * g)
{
	if (p)
		return p->checkClose(g);
	
	if (open == 0)
		g->roadUnclosed(this);
}

void CloisterNode::checkClose(Game *g)
{
	if (p)
		return p->checkClose(g);
	
	if (surroundingTiles == 9)
		g->cloisterClosed(this);
}

void CloisterNode::checkUnclose(Game * g)
{
	if (p)
		return p->checkClose(g);
	
	if (surroundingTiles == 9)
		g->cloisterClosed(this);
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
	qDebug() << "connect tile:" << id << "<->" << other->id;
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
		
		QString st = QString("%1").arg(thisNode->id);
		for (Node * n = thisNode->p; n != 0; n = n->p)
			st = QString("%1 -> %2").arg(st).arg(n->id);
		QString so = QString("%1").arg(otherNode->id);
		for (Node * n = otherNode->p; n != 0; n = n->p)
			so = QString("%1 -> %2").arg(so).arg(n->id);
		qDebug() << "   bevore thisNode :" << st;
		qDebug() << "   bevore otherNode:" << so;
		
		thisNode->connect(otherNode, game);
		
		st = QString("%1").arg(thisNode->id);
		for (Node * n = thisNode->p; n != 0; n = n->p)
			st = QString("%1 -> %2").arg(st).arg(n->id);
		so = QString("%1").arg(otherNode->id);
		for (Node * n = otherNode->p; n != 0; n = n->p)
			so = QString("%1 -> %2").arg(so).arg(n->id);
		qDebug() << "   after  thisNode :" << st;
		qDebug() << "   after  otherNode:" << so;
	}
	
	if (cloister != 0)
		cloister->addSurroundingTile(game);
	if (other->cloister != 0)
		other->cloister->addSurroundingTile(game);
}

void Tile::disconnect(Tile::Side side, Tile * other, Game * game)
{
	qDebug() << "disconnect tile:" << id << "<->" << other->id;
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
//		EdgeType otherNode = otherNodeList[nodeCount - i - 1];
//		EdgeType thisNode = nodeList[i];
		EdgeType otherNode = otherNodeList[i];
		EdgeType thisNode = nodeList[nodeCount - i - 1];
		
		QString st = QString("%1").arg(thisNode->id);
		for (Node * n = thisNode->p; n != 0; n = n->p)
			st = QString("%1 -> %2").arg(st).arg(n->id);
		QString so = QString("%1").arg(otherNode->id);
		for (Node * n = otherNode->p; n != 0; n = n->p)
			so = QString("%1 -> %2").arg(so).arg(n->id);
		qDebug() << "   bevore thisNode :" << st;
		qDebug() << "   bevore otherNode:" << so;
		
		thisNode->disconnect(otherNode, game);
		
		st = QString("%1").arg(thisNode->id);
		for (Node * n = thisNode->p; n != 0; n = n->p)
			st = QString("%1 -> %2").arg(st).arg(n->id);
		so = QString("%1").arg(otherNode->id);
		for (Node * n = otherNode->p; n != 0; n = n->p)
			so = QString("%1 -> %2").arg(so).arg(n->id);
		qDebug() << "   after  thisNode :" << st;
		qDebug() << "   after  otherNode:" << so;
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
		nodeMap[nodes[i]] = copy->nodes[i];
	}
	
	for (int i = 0; i < 4; ++i)
	{
		int const enc = edgeNodeCount(edges[i]);
		for (int j = 0; j < enc; ++j)
			copy->setEdgeNode((Side)i, j, nodeMap[edgeNodes[(Side)i][j]]);
	}
	
	for (int i = 0; i < nodeCount; ++i)
	{
		switch (nodes[i]->t)
		{
			case Field:
			{
				FieldNode * tf = static_cast<FieldNode *>(nodes[i]);
				FieldNode * cf = static_cast<FieldNode *>(copy->nodes[i]);
				for (CityNode * c : tf->cities)
					cf->cities.push_back(static_cast<CityNode *>(nodeMap[c]));
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
	edgeNodes[side][index] = n;
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
