#include "simpleplayer3.h"
#include "randomplayer.h"

void SimplePlayer3::newGame(int /*player*/, const Game * game)
{
	SimplePlayer3::game = game;
	board = game->getBoard();
}

void SimplePlayer3::playerMoved(int /*player*/, const Tile * /*tile*/, const MoveHistoryEntry & /*move*/)
{
}

void SimplePlayer3::undoneMove(const MoveHistoryEntry & /*move*/)
{
}

TileMove SimplePlayer3::getTileMove(int player, const Tile * tile, const MoveHistoryEntry & move, const TileMovesType & possible)
{
	meepleMoveSet = false;
#if SIMPLE_PLAYER3_TILE_RANDOM > 0
	if (r.nextInt(100) < SIMPLE_PLAYER3_TILE_RANDOM)
		return RandomPlayer::instance.getTileMove(player, tile, move, possible);
#endif

	static constexpr int dx[8] = {-1,  0, +1,  0, -1, +1, +1, -1};
	static constexpr int dy[8] = { 0, -1,  0, +1, -1, -1, +1, +1};
	static constexpr Tile::Side dir[4]    = {Tile::left,  Tile::up,    Tile::right, Tile::down};
	static constexpr Tile::Side oppDir[4] = {Tile::right, Tile::down,  Tile::left,  Tile::up  };
	//                                                     { None = 0, Field, City, Road, Cloister }
#if SIMPLE_PLAYER3_RULE_FIELD
	static constexpr int terrainBonus[TERRAIN_TYPE_SIZE] = {        0,     1,    6,    2,        1 };
#else
	// I don't understand why, but this works better, if fields are disabled.
	static constexpr int terrainBonus[TERRAIN_TYPE_SIZE] = {        0,     0,    3,    1,        1 };
#endif

	int const playerCount = game->getPlayerCount();
	int const myBonus = playerCount * 10;
	int const opponentBonus = (myBonus - 1) * 10;
	int const meepleCount = game->getPlayerMeeples(player);
	bool const hasMeeples = (meepleCount > 0);

#if SIMPLE_PLAYER3_USE_MEEPLE_PENALTY
	static constexpr int meeplePenalties[MEEPLE_COUNT+1] = {24, 22, 14, 8, 5, 3, 2, 1};		//From SimplePlayer2
	int const meeplePenalty = meeplePenalties[meepleCount];
#else
	static constexpr int meeplePenalty = 0;
#endif

#if SIMPLE_PLAYER3_USE_OPEN_PENALTY
	static constexpr int openPenalty = 1;
#else
	static constexpr int openPenalty = 0;
#endif


	VarLengthArrayWrapper<Move, TILE_ARRAY_LENGTH * NODE_ARRAY_LENGTH>::type goodMoves;
	int best = std::numeric_limits<int>::min();

#if  SIMPLE_PLAYER3_RULE_ROAD_CITY || SIMPLE_PLAYER3_RULE_FIELD || SIMPLE_PLAYER3_RULE_CLOISTER_1
	uchar const nodeCount = tile->getNodeCount();
	for (TileMove const & tileMove : possible)
	{
		int points = 0;
		VarLengthArrayWrapper<MeepleMove, TILE_ARRAY_LENGTH>::type meepleMoves(1);
		int bestMeeplePoints = 0;
		for (uchar nodeIndex = 0; nodeIndex < nodeCount; ++nodeIndex)
		{
			Node const * node = tile->getNode(nodeIndex);
			TerrainType const & terrain = node->getTerrain();
			int nodePoints = 0;

			int score = node->getScore();
			int meeples[MAX_PLAYERS] = {};
			int maxMeeples = 0;

			switch (terrain)
			{
				case None:
					break;
				case Cloister:
				{
#if SIMPLE_PLAYER3_RULE_CLOISTER_1
					for (int i = 0; i < 8; ++i)
					{
						if (board->getTile(tileMove.x + dx[i], tileMove.y + dy[i]) != 0)
							++score;
					}
					break;
#else
					continue;
#endif
				}
				case Field:
				{
#if SIMPLE_PLAYER3_RULE_FIELD
					for (int i = 0; i < 4; ++i)
					{
						Tile const * otherTile = board->getTile(tileMove.x + dx[i], tileMove.y + dy[i]);
						if (otherTile == 0)
							continue;
						Tile::Side const & direction = dir[i];
						Tile::Side const & oppDirection = oppDir[i];
						for (int j = 0; j < EDGE_NODE_COUNT; ++j)
						{
							if (tile->getEdgeNode(direction, j, tileMove.orientation) != node)
								continue;

							Node const * otherNode = otherTile->getEdgeNode(oppDirection, EDGE_NODE_COUNT-1 - j);
							Q_ASSERT(otherNode != 0);
							Q_ASSERT(otherNode->getTerrain() == terrain);

							score = (score + otherNode->getScore()) / 2;	//Fields usually don't add up in score, if merged.
							for (int p = 0; p < playerCount; ++p)
							{
								meeples[p] += otherNode->getPlayerMeeples(p);
								if (meeples[p] > maxMeeples)
									maxMeeples = meeples[p];
							}
						}
					}
					break;
#else
					continue;
#endif
				}
				case Road:
				case City:
				{
#if  SIMPLE_PLAYER3_RULE_ROAD_CITY
					for (int i = 0; i < 4; ++i)
					{
						Tile::Side const & direction = dir[i];
						if (tile->getFeatureNode(direction, tileMove.orientation) != node)
							continue;

						Tile const * otherTile = board->getTile(tileMove.x + dx[i], tileMove.y + dy[i]);
						if (otherTile == 0)
							continue;
						Tile::Side const & oppDirection = oppDir[i];
						Q_ASSERT(otherTile->getEdge(oppDirection) == terrain);
						Node const * otherNode = otherTile->getFeatureNode(oppDirection);
						Q_ASSERT(otherNode != 0);
						Q_ASSERT(otherNode->getTerrain() == terrain);

						score += otherNode->getScore();
						for (int p = 0; p < playerCount; ++p)
						{
							meeples[p] += otherNode->getPlayerMeeples(p);
							if (meeples[p] > maxMeeples)
								maxMeeples = meeples[p];
						}
					}
					break;
#else
					continue;
#endif
				}
			}

			if (terrain == Cloister)
			{
				if (hasMeeples)
				{
					int const meeplePoints = (myBonus * score) * terrainBonus[terrain]; // no meeplePenalty
					if (meeplePoints > bestMeeplePoints)
					{
						bestMeeplePoints = meeplePoints;
						meepleMoves.clear();
						meepleMoves.push_back(MeepleMove{nodeIndex});
					}
					else if (meeplePoints == bestMeeplePoints)
					{
						meepleMoves.push_back(MeepleMove{nodeIndex});
					}
				}
			}
			else
			{
				if (maxMeeples != 0) //occupied
				{
					for (int p = 0; p < playerCount; ++p)
					{
						if (meeples[p] != maxMeeples)
							continue;
						if (p == player)
							nodePoints += myBonus * score;
						else
							nodePoints -= opponentBonus * score;
					}
				}
				else
				{
					nodePoints -= openPenalty * score;

					if (hasMeeples)
					{
						int const meeplePoints = (myBonus * score - meeplePenalty) * terrainBonus[terrain];
						if (meeplePoints > bestMeeplePoints)
						{
							bestMeeplePoints = meeplePoints;
							meepleMoves.clear();
							meepleMoves.push_back(MeepleMove{nodeIndex});
						}
						else if (meeplePoints == bestMeeplePoints)
						{
							meepleMoves.push_back(MeepleMove{nodeIndex});
						}
					}
				}
			}
			points += nodePoints * terrainBonus[terrain];
		}

		points += bestMeeplePoints;
		if (points > best)
		{
			best = points;
			goodMoves.clear();
			for (MeepleMove const & mm : meepleMoves)
				goodMoves.push_back(Move{tileMove, mm});
		}
		else if (points == best)
		{
			for (MeepleMove const & mm : meepleMoves)
				goodMoves.push_back(Move{tileMove, mm});
		}
	}

	Move const & m = goodMoves[r.nextInt(goodMoves.size())];
	meepleMove = m.meepleMove;
	meepleMoveSet = true;
	return m.tileMove;
#endif
	return RandomPlayer::instance.getTileMove(player, tile, move, possible);
}

MeepleMove SimplePlayer3::getMeepleMove(int player, const Tile * tile, const MoveHistoryEntry & move, const MeepleMovesType & possible)
{
#if SIMPLE_PLAYER3_MEEPLE_RANDOM > 0
	if (r.nextInt(100) < SIMPLE_PLAYER3_MEEPLE_RANDOM)
		return RandomPlayer::instance.getMeepleMove(player, tile, move, possible);
#endif

	if (meepleMoveSet)
	{
		meepleMoveSet = false;
#if SIMPLE_PLAYER3_RULE_FIELD
		if (std::find(possible.cbegin(), possible.cend(), meepleMove) == possible.cend())	// This is needed, because I cannot figure out if two fileds will be the same without simulating the move.
			return RandomPlayer::instance.getMeepleMove(player, tile, move, possible);
#else
		Q_ASSERT(std::find(possible.cbegin(), possible.cend(), meepleMove) != possible.cend());;
#endif
		return meepleMove;
	}
	return RandomPlayer::instance.getMeepleMove(player, tile, move, possible);
}

void SimplePlayer3::endGame()
{
}

QString SimplePlayer3::getTypeName() const
{
	return "SimplePlayer3";
}

Player *SimplePlayer3::clone() const
{
	return new SimplePlayer3();
}
