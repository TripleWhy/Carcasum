#include "simpleplayer2.h"
#include "randomplayer.h"

void SimplePlayer2::newGame(int /*player*/, const Game * game)
{
	SimplePlayer2::game = game;
	board = game->getBoard();
}

void SimplePlayer2::playerMoved(int /*player*/, const Tile * /*tile*/, const MoveHistoryEntry & /*move*/)
{
}

TileMove SimplePlayer2::getTileMove(int player, const Tile * tile, const MoveHistoryEntry & move, const TileMovesType & possible)
{
	meepleMoveSet = false;
#if SIMPLE_PLAYER2_TILE_RANDOM > 0
	if (r.nextInt(100) < SIMPLE_PLAYER2_TILE_RANDOM)
		return RandomPlayer::instance.getTileMove(player, tile, move, possible);
#endif

	static constexpr int dx[8] = {-1,  0, +1,  0, -1, +1, +1, -1};
	static constexpr int dy[8] = { 0, -1,  0, +1, -1, -1, +1, +1};
	static constexpr Tile::Side dir[4]    = {Tile::left,  Tile::up,    Tile::right, Tile::down};
	static constexpr Tile::Side oppDir[4] = {Tile::right, Tile::down,  Tile::left,  Tile::up  };
	//                                                     { None = 0, Field, City, Road, Cloister }
#if SIMPLE_PLAYER2_RULE_FIELD
	static constexpr int terrainBonus[TERRAIN_TYPE_SIZE] = {        0,     1,    6,    2,        0 };
#else
	// I don't understand why, but this works better, if fields are disabled.
	static constexpr int terrainBonus[TERRAIN_TYPE_SIZE] = {        0,     0,    3,    1,        0 };
#endif

	int const playerCount = game->getPlayerCount();
	int const myBonus = playerCount * 10;
	int const opponentBonus = (myBonus - 1) * 10;
	int const meepleCount = game->getPlayerMeeples(player);
	bool const hasMeeples = (meepleCount > 0);

#if SIMPLE_PLAYER2_USE_MEEPLE_PENALTY
//	static constexpr int meeplePenalties[MEEPLE_COUNT+1] = {5, 4, 3, 2, 1, 1, 0, 0};	//original
//	static constexpr int meeplePenalties[MEEPLE_COUNT+1] = {13, 8, 5, 3, 2, 1, 1, 0};	//also good
//	static constexpr int meeplePenalties[MEEPLE_COUNT+1] = {29, 22, 16, 11, 7, 4, 2, 1};	//5174
	static constexpr int meeplePenalties[MEEPLE_COUNT+1] = {24, 22, 14, 8, 5, 3, 2, 1};		//5303
//	static constexpr int meeplePenalties[MEEPLE_COUNT+1] = {41, 24, 14, 8, 5, 3, 2, 1};		//5205
//	static constexpr int meeplePenalties[MEEPLE_COUNT+1] = {27, 17, 10, 7, 4, 3, 2, 1};		//4785
	int const meeplePenalty = meeplePenalties[meepleCount];
#else
	static constexpr int meeplePenalty = 0;
#endif

#if SIMPLE_PLAYER2_USE_OPEN_PENALTY
	static constexpr int openPenalty = 1;
#else
	static constexpr int openPenalty = 0;
#endif

	if (tile->getCloisterNode() != 0)
	{
		if (hasMeeples)
		{
#if SIMPLE_PLAYER2_RULE_CLOISTER_1
			VarLengthArrayWrapper<TileMove const *, TILE_ARRAY_LENGTH>::type goodMoves;
			int best = 0;
			for (TileMove const & tileMove : possible)
			{
				int surroundingTiles = 0;
				for (int i = 0; i < 8; ++i)
				{
					if (board->getTile(tileMove.x + dx[i], tileMove.y + dy[i]) != 0)
						++surroundingTiles;
				}
				if (surroundingTiles > best)
				{
					best = surroundingTiles;
					goodMoves.clear();
					goodMoves.push_back(&tileMove);
				}
				else if (surroundingTiles == best)
				{
					goodMoves.push_back(&tileMove);
				}
			}

			for (uchar i = 0, c = tile->getNodeCount(); i < c; ++i)
			{
				if (tile->getNode(i)->getTerrain() == Cloister)
				{
					meepleMove.nodeIndex = i;
					meepleMoveSet = true;
					break;
				}
			}

			return *goodMoves[r.nextInt(goodMoves.size())];
#endif
		}
		else
		{
#if SIMPLE_PLAYER2_RULE_CLOISTER_2
			VarLengthArrayWrapper<TileMove const *, TILE_ARRAY_LENGTH>::type goodMoves;
			int best = std::numeric_limits<int>::min();
			for (TileMove const & tileMove : possible)
			{
				int points = 0;
				for (int i = 0; i < 8; ++i)
				{
					Tile const * t = board->getTile(tileMove.x + dx[i], tileMove.y + dy[i]);
					if (t != 0)
					{
						Node const * n = t->getCloisterNode();
						if (n != 0 && n->getMaxMeeples() != 0)
						{
							if (n->getPlayerMeeples(player) != 0)
								points += myBonus;
							else
								points -= opponentBonus;
						}
					}
				}
				if (points > best)
				{
					best = points;
					goodMoves.clear();
					goodMoves.push_back(&tileMove);
				}
				else if (points == best)
				{
					goodMoves.push_back(&tileMove);
				}
			}

			return *goodMoves[r.nextInt(goodMoves.size())];
#endif
		}
	}
	else // No cloister tile
	{
#if  SIMPLE_PLAYER2_RULE_ROAD_CITY || SIMPLE_PLAYER2_RULE_FIELD
		VarLengthArrayWrapper<Move, TILE_ARRAY_LENGTH * NODE_ARRAY_LENGTH>::type goodMoves;
		int best = std::numeric_limits<int>::min();

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

				int score = node->getScore();
				int meeples[MAX_PLAYERS] = {};
				int maxMeeples = 0;

				if (terrain == Field)
				{
#if SIMPLE_PLAYER2_RULE_FIELD
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

							score = qMax(score, otherNode->getScore());	//Fields usually don't add up in score, if merged. Max is also not correct, but it's ok as an estimate.
							for (int p = 0; p < playerCount; ++p)
							{
								meeples[p] += otherNode->getPlayerMeeples(p);
								if (meeples[p] > maxMeeples)
									maxMeeples = meeples[p];
							}
						}
					}
#else
					continue;
#endif
				}
				else
				{
#if  SIMPLE_PLAYER2_RULE_ROAD_CITY
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
#else
					continue;
#endif
				}

				if (maxMeeples != 0) //occupied
				{
					for (int p = 0; p < playerCount; ++p)
					{
						if (meeples[p] != maxMeeples)
							continue;
						if (p == player)
							points += myBonus * score;
						else
							points -= opponentBonus * score;
					}
				}
				else
				{
					points -= openPenalty * score;

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
				points *= terrainBonus[terrain];
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
	}
	return RandomPlayer::instance.getTileMove(player, tile, move, possible);
}

MeepleMove SimplePlayer2::getMeepleMove(int player, const Tile * tile, const MoveHistoryEntry & move, const MeepleMovesType & possible)
{
#if SIMPLE_PLAYER2_MEEPLE_RANDOM > 0
	if (r.nextInt(100) < SIMPLE_PLAYER2_MEEPLE_RANDOM)
		return RandomPlayer::instance.getMeepleMove(player, tile, move, possible);
#endif

	if (meepleMoveSet)
	{
#if SIMPLE_PLAYER2_RULE_FIELD
		if (std::find(possible.cbegin(), possible.cend(), meepleMove) == possible.cend())	// This is needed, because I cannot figure out if two fileds will be the same without simulating the move.
			return RandomPlayer::instance.getMeepleMove(player, tile, move, possible);
#else
		Q_ASSERT(std::find(possible.cbegin(), possible.cend(), meepleMove) != possible.cend());;
#endif
		return meepleMove;
	}
	return RandomPlayer::instance.getMeepleMove(player, tile, move, possible);
}

void SimplePlayer2::endGame()
{
}

QString SimplePlayer2::getTypeName()
{
	return "SimplePlayer2";
}

Player *SimplePlayer2::clone() const
{
	return new SimplePlayer2();
}
