#include "tile.h"

#include <QDebug>

Tile::Tile(TileSet tileSet, int tileType, TerrainType const edges[4])
	: edges(edges),
	  tileSet(tileSet),
	  tileType(tileType)
{
	edgeNodes[left] = createEdgeList(edges[left]);
	edgeNodes[up] = createEdgeList(edges[up]);
	edgeNodes[right] = createEdgeList(edges[right]);
	edgeNodes[down] = createEdgeList(edges[down]);
}

Tile::Tile(const Tile & t)
	: edges(t.edges),
	  tileSet(t.tileSet),
	  tileType(t.tileType)
{
	//TODO
}

Tile::~Tile()
{
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

	Node ** nodeList = getEdgeNodes(side);
	Node ** otherNodeList = other->getEdgeNodes(otherSide);
	int const nodeCount = edgeNodeCount(t);
	for (int i = 0; i < nodeCount; ++i)
	{
		Node * otherNode = otherNodeList[i];
		Node * thisNode = nodeList[nodeCount - i - 1];

		if (thisNode->connect(otherNode))
		{
			other->nodes.removeOne(otherNode);
			otherNodeList[i] = thisNode;
			other->nodes.append(thisNode);
		}
	}

	return true;
}

Node ** Tile::getEdgeNodes(Tile::Side side)
{
	return edgeNodes[(4 + side - orientation) % 4];
}

Node **Tile::createEdgeList(TerrainType t)
{
	switch (t)
	{
		case Field:
		case City:
			return new Node*[1];
		case Road:
			return new Node*[3];
		case None:
		case Cloister:
			break;
	}
	return 0;
}



QList<Tile *> TileFactory::createTiles(Tile::TileSets s)
{
	QList<Tile *> tiles;
	if (s.testFlag(Tile::BaseGame))
	{
		int const tileTypes = 24;
		//                            A  B  C  D  E  F
		int tileCounts[tileTypes] = { 2, 4, 1, 3, 5, 2,
									  3, 3, 1, 3, 3, 3,
									  2, 3, 2, 3, 1, 3,
									  2, 1, 8, 9, 4, 1 };

		tiles.append(createTile(Tile::BaseGame, 3));
		for (int i = 0; i < tileTypes; ++i)
		{
			int count = tileCounts[i];
			for (int j = 0; j < count; ++j)
				tiles.append(createTile(Tile::BaseGame, i));
		}
		Q_ASSERT(tiles.size() != 72);
	}
	return tiles;
}

inline Node * newFieldNode()
{
	return new Node(Field);
}

inline Node * newCityNode(uchar open, uchar score = 1)
{
	return new CityNode(open, score);
}

inline Node * newRoadNode()
{
	return new Node(Road);
}

inline Node * newCloisterNode()
{
	return new Node(Cloister);
}

Tile * TileFactory::createTile(Tile::TileSet s, int type)
{
	switch (s)
	{
		case Tile::BaseGame:
		{
			switch (type)
			{
				case  5: //F
				case 12: //M
				case 14: //O
				case 16: //Q
				case 18: //S
				{
					// quick and dirty
					Tile * t = createTile(s, type + 1);
					static_cast<CityNode*>(t->edgeNodes[Tile::up][0])->score = 2;
					return t;
				}
				case 0: //A
				{
					static TerrainType e[4] = { Field, Field, Field, Road };
					Node * cloister = newCloisterNode();
					Node * road = newRoadNode();
					Node * field = newFieldNode();

					Tile * t = new Tile(s, type, e);
					t->nodes.append(cloister);
					t->nodes.append(road);
					t->nodes.append(field);

					t->edgeNodes[Tile::left][0] = field;
					t->edgeNodes[Tile::up][0] = field;
					t->edgeNodes[Tile::right][0] = field;
					t->edgeNodes[Tile::down][0] = field;
					t->edgeNodes[Tile::down][1] = road;
					t->edgeNodes[Tile::down][2] = field;

					return t;
				}
				case 1: //B
				{
					static TerrainType e[4] = { Field, Field, Field, Field };
					Node * cloister = newCloisterNode();
					Node * field = newFieldNode();

					Tile * t = new Tile(s, type, e);
					t->nodes.append(cloister);
					t->nodes.append(field);

					t->edgeNodes[Tile::left][0] = field;
					t->edgeNodes[Tile::up][0] = field;
					t->edgeNodes[Tile::right][0] = field;
					t->edgeNodes[Tile::down][0] = field;

					return t;
				}
				case 2: //C
				{
					static TerrainType e[4] = { City, City, City, City };
					Node * city = newCityNode(4, 2);

					Tile * t = new Tile(s, type, e);
					t->nodes.append(city);

					for (int i = 0; i < 4; i++)
						t->edgeNodes[i][0] = city;

					return t;
				}
				case 3: //D
				{
					static TerrainType e[4] = { Field, Road, City, Road };
					Node * city = newCityNode(1);
					Node * road = newRoadNode();
					Node * field1 = newFieldNode();
					Node * field2 = newFieldNode();

					Tile * t = new Tile(s, type, e);
					t->nodes.append(city);
					t->nodes.append(road);
					t->nodes.append(field1);
					t->nodes.append(field2);

					t->edgeNodes[Tile::left][0] = field1;
					t->edgeNodes[Tile::up][0] = field2;
					t->edgeNodes[Tile::up][1] = road;
					t->edgeNodes[Tile::up][2] = field1;
					t->edgeNodes[Tile::right][0] = city;
					t->edgeNodes[Tile::down][0] = field1;
					t->edgeNodes[Tile::down][1] = road;
					t->edgeNodes[Tile::down][2] = field2;

					return t;
				}
				case 4: //E
				{
					static TerrainType e[4] = { Field, City, Field, Field };
					Node * city = newCityNode(1);
					Node * field = newFieldNode();

					Tile * t = new Tile(s, type, e);
					t->nodes.append(city);
					t->nodes.append(field);

					t->edgeNodes[Tile::left][0] = field;
					t->edgeNodes[Tile::up][0] = city;
					t->edgeNodes[Tile::right][0] = field;
					t->edgeNodes[Tile::down][0] = field;

					return t;
				}
				case 6: //G
				{
					static TerrainType e[4] = { City, Field, City, Field };
					Node * city = newCityNode(2);
					Node * field1 = newFieldNode();
					Node * field2 = newFieldNode();

					Tile * t = new Tile(s, type, e);
					t->nodes.append(city);
					t->nodes.append(field1);
					t->nodes.append(field2);

					t->edgeNodes[Tile::left][0] = city;
					t->edgeNodes[Tile::up][0] = field1;
					t->edgeNodes[Tile::right][0] = city;
					t->edgeNodes[Tile::down][0] = field2;

					return t;
				}
				case 7: //H
				{
					static TerrainType e[4] = { City, Field, City, Field };
					Node * city1 = newCityNode(1);
					Node * city2 = newCityNode(1);
					Node * field = newFieldNode();

					Tile * t = new Tile(s, type, e);
					t->nodes.append(city1);
					t->nodes.append(city2);
					t->nodes.append(field);

					t->edgeNodes[Tile::left][0] = city2;
					t->edgeNodes[Tile::up][0] = field;
					t->edgeNodes[Tile::right][0] = city1;
					t->edgeNodes[Tile::down][0] = field;

					return t;
				}
				case 8: //I
				{
					static TerrainType e[4] = { Field, Field, City, City };
					Node * city1 = newCityNode(1);
					Node * city2 = newCityNode(1);
					Node * field = newFieldNode();

					Tile * t = new Tile(s, type, e);
					t->nodes.append(city1);
					t->nodes.append(city2);
					t->nodes.append(field);

					t->edgeNodes[Tile::left][0] = field;
					t->edgeNodes[Tile::up][0] = field;
					t->edgeNodes[Tile::right][0] = city1;
					t->edgeNodes[Tile::down][0] = city2;

					return t;
				}
				case 9: //J
				{
					static TerrainType e[4] = { Field, City, Road, Road };
					Node * city = newCityNode(1);
					Node * road = newRoadNode();
					Node * field1 = newFieldNode();
					Node * field2 = newFieldNode();

					Tile * t = new Tile(s, type, e);
					t->nodes.append(city);
					t->nodes.append(road);
					t->nodes.append(field1);
					t->nodes.append(field2);

					t->edgeNodes[Tile::left][0] = field1;
					t->edgeNodes[Tile::up][0] = city;
					t->edgeNodes[Tile::right][0] = field2;
					t->edgeNodes[Tile::right][1] = road;
					t->edgeNodes[Tile::right][2] = field1;
					t->edgeNodes[Tile::down][0] = field1;
					t->edgeNodes[Tile::down][1] = road;
					t->edgeNodes[Tile::down][2] = field2;

					return t;
				}
				case 10: //K
				{
					static TerrainType e[4] = { Road, Road, City, Field };
					Node * city = newCityNode(1);
					Node * road = newRoadNode();
					Node * field1 = newFieldNode();
					Node * field2 = newFieldNode();

					Tile * t = new Tile(s, type, e);
					t->nodes.append(city);
					t->nodes.append(road);
					t->nodes.append(field1);
					t->nodes.append(field2);

					t->edgeNodes[Tile::left][0] = field1;
					t->edgeNodes[Tile::left][1] = road;
					t->edgeNodes[Tile::left][2] = field2;
					t->edgeNodes[Tile::up][0] = field2;
					t->edgeNodes[Tile::up][1] = road;
					t->edgeNodes[Tile::up][2] = field1;
					t->edgeNodes[Tile::right][0] = city;
					t->edgeNodes[Tile::down][0] = field1;

					return t;
				}
				case 11: //L
				{
					static TerrainType e[4] = { Road, Road, City, Road };
					Node * city = newCityNode(1);
					Node * road1 = newRoadNode();
					Node * road2 = newRoadNode();
					Node * road3 = newRoadNode();
					Node * field1 = newFieldNode();
					Node * field2 = newFieldNode();
					Node * field3 = newFieldNode();

					Tile * t = new Tile(s, type, e);
					t->nodes.append(city);
					t->nodes.append(road1);
					t->nodes.append(road2);
					t->nodes.append(road3);
					t->nodes.append(field1);
					t->nodes.append(field2);
					t->nodes.append(field3);

					t->edgeNodes[Tile::left][0] = field3;
					t->edgeNodes[Tile::left][1] = road3;
					t->edgeNodes[Tile::left][2] = field2;
					t->edgeNodes[Tile::up][0] = field1;
					t->edgeNodes[Tile::up][1] = road1;
					t->edgeNodes[Tile::up][2] = field3;
					t->edgeNodes[Tile::right][0] = city;
					t->edgeNodes[Tile::down][0] = field2;
					t->edgeNodes[Tile::down][1] = road2;
					t->edgeNodes[Tile::down][2] = field1;

					return t;
				}
				case 13: //N
				{
					static TerrainType e[4] = { City, City, Field, Field };
					Node * city = newCityNode(2);
					Node * field = newFieldNode();

					Tile * t = new Tile(s, type, e);
					t->nodes.append(city);
					t->nodes.append(field);

					t->edgeNodes[Tile::left][0] = city;
					t->edgeNodes[Tile::up][0] = city;
					t->edgeNodes[Tile::right][0] = field;
					t->edgeNodes[Tile::down][0] = field;

					return t;
				}
				case 15: //P
				{
					static TerrainType e[4] = { City, City, Road, Road };
					Node * city = newCityNode(2);
					Node * road = newRoadNode();
					Node * field1 = newFieldNode();
					Node * field2 = newFieldNode();

					Tile * t = new Tile(s, type, e);
					t->nodes.append(city);
					t->nodes.append(road);
					t->nodes.append(field1);
					t->nodes.append(field2);

					t->edgeNodes[Tile::left][0] = city;
					t->edgeNodes[Tile::up][0] = city;
					t->edgeNodes[Tile::right][0] = field2;
					t->edgeNodes[Tile::right][1] = road;
					t->edgeNodes[Tile::right][2] = field1;
					t->edgeNodes[Tile::down][0] = field1;
					t->edgeNodes[Tile::down][1] = road;
					t->edgeNodes[Tile::down][2] = field2;

					return t;
				}
				case 17: //R
				{
					static TerrainType e[4] = { City, City, City, Field };
					Node * field = newFieldNode();
					Node * city = newCityNode(3);

					Tile * t = new Tile(s, type, e);
					t->nodes.append(field);
					t->nodes.append(city);

					t->edgeNodes[Tile::left][0] = city;
					t->edgeNodes[Tile::up][0] = city;
					t->edgeNodes[Tile::right][0] = city;
					t->edgeNodes[Tile::down][0] = field;

					return t;
				}
				case 19: //T
				{
					static TerrainType e[4] = { City, City, City, Road };
					Node * city = newCityNode(3);
					Node * road = newRoadNode();
					Node * field1 = newFieldNode();
					Node * field2 = newFieldNode();

					Tile * t = new Tile(s, type, e);
					t->nodes.append(city);
					t->nodes.append(road);
					t->nodes.append(field1);
					t->nodes.append(field2);

					t->edgeNodes[Tile::left][0] = city;
					t->edgeNodes[Tile::up][0] = city;
					t->edgeNodes[Tile::right][0] = city;
					t->edgeNodes[Tile::down][0] = field1;
					t->edgeNodes[Tile::down][1] = road;
					t->edgeNodes[Tile::down][2] = field2;

					return t;
				}
				case 20: //U
				{
					static TerrainType e[4] = { Field, Road, Field, Road };
					Node * road = newRoadNode();
					Node * field1 = newFieldNode();
					Node * field2 = newFieldNode();

					Tile * t = new Tile(s, type, e);
					t->nodes.append(road);
					t->nodes.append(field1);
					t->nodes.append(field2);

					t->edgeNodes[Tile::left][0] = field1;
					t->edgeNodes[Tile::up][0] = field2;
					t->edgeNodes[Tile::up][1] = road;
					t->edgeNodes[Tile::up][2] = field1;
					t->edgeNodes[Tile::right][0] = field2;
					t->edgeNodes[Tile::down][0] = field1;
					t->edgeNodes[Tile::down][1] = road;
					t->edgeNodes[Tile::down][2] = field2;

					return t;
				}
				case 21: //V
				{
					static TerrainType e[4] = { Road, Field, Field, Road };
					Node * road = newRoadNode();
					Node * field1 = newFieldNode();
					Node * field2 = newFieldNode();

					Tile * t = new Tile(s, type, e);
					t->nodes.append(road);
					t->nodes.append(field1);
					t->nodes.append(field2);

					t->edgeNodes[Tile::left][0] = field1;
					t->edgeNodes[Tile::left][1] = road;
					t->edgeNodes[Tile::left][2] = field2;
					t->edgeNodes[Tile::up][0] = field1;
					t->edgeNodes[Tile::right][0] = field1;
					t->edgeNodes[Tile::down][0] = field2;
					t->edgeNodes[Tile::down][1] = road;
					t->edgeNodes[Tile::down][2] = field1;

					return t;
				}
				case 22: //W
				{
					static TerrainType e[4] = { Road, Field, Road, Road };
					Node * road1 = newRoadNode();
					Node * road2 = newRoadNode();
					Node * road3 = newRoadNode();
					Node * field1 = newFieldNode();
					Node * field2 = newFieldNode();
					Node * field3 = newFieldNode();

					Tile * t = new Tile(s, type, e);
					t->nodes.append(road1);
					t->nodes.append(road2);
					t->nodes.append(road3);
					t->nodes.append(field1);
					t->nodes.append(field2);
					t->nodes.append(field3);

					t->edgeNodes[Tile::left][0] = field1;
					t->edgeNodes[Tile::left][1] = road1;
					t->edgeNodes[Tile::left][2] = field3;
					t->edgeNodes[Tile::up][0] = field1;
					t->edgeNodes[Tile::right][0] = field2;
					t->edgeNodes[Tile::right][1] = road2;
					t->edgeNodes[Tile::right][2] = field1;
					t->edgeNodes[Tile::down][0] = field3;
					t->edgeNodes[Tile::down][1] = road3;
					t->edgeNodes[Tile::down][2] = field2;

					return t;
				}
				case 23: //X
				{
					static TerrainType e[4] = { Road, Road, Road, Road };
					Node * road1 = newRoadNode();
					Node * road2 = newRoadNode();
					Node * road3 = newRoadNode();
					Node * road4 = newRoadNode();
					Node * field1 = newFieldNode();
					Node * field2 = newFieldNode();
					Node * field3 = newFieldNode();
					Node * field4 = newFieldNode();

					Tile * t = new Tile(s, type, e);
					t->nodes.append(road1);
					t->nodes.append(road2);
					t->nodes.append(road3);
					t->nodes.append(road4);
					t->nodes.append(field1);
					t->nodes.append(field2);
					t->nodes.append(field3);
					t->nodes.append(field4);

					t->edgeNodes[Tile::left][0] = field1;
					t->edgeNodes[Tile::left][1] = road1;
					t->edgeNodes[Tile::left][2] = field4;
					t->edgeNodes[Tile::up][0] = field2;
					t->edgeNodes[Tile::up][1] = road2;
					t->edgeNodes[Tile::up][2] = field1;
					t->edgeNodes[Tile::right][0] = field3;
					t->edgeNodes[Tile::right][1] = road3;
					t->edgeNodes[Tile::right][2] = field2;
					t->edgeNodes[Tile::down][0] = field4;
					t->edgeNodes[Tile::down][1] = road4;
					t->edgeNodes[Tile::down][2] = field3;

					return t;
				}
			}
			break;
		}
	}
	return 0;
}
