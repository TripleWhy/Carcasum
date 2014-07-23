/*
	This file is part of Carcasum.

	Carcasum is free software: you can redistribute it and/or modify
	it under the terms of the GNU Affero General Public License as published by
	the Free Software Foundation, either version 3 of the License, or
	(at your option) any later version.

	Carcasum is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU Affero General Public License for more details.

	You should have received a copy of the GNU Affero General Public License
	along with Carcasum.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "simpleplayer.h"
#include "randomplayer.h"

void SimplePlayer::newGame(int /*player*/, const Game * game)
{
	SimplePlayer::game = game;
	board = game->getBoard();
}

void SimplePlayer::playerMoved(int /*player*/, const Tile * /*tile*/, const MoveHistoryEntry & /*move*/)
{
}

void SimplePlayer::undoneMove(const MoveHistoryEntry & /*move*/)
{
}

#define SIMPLE_PLAYER_RULE_CLOISTER_1    1
#define SIMPLE_PLAYER_RULE_CLOISTER_2    1
#define SIMPLE_PLAYER_RULE_ROAD_CITY     1
#define SIMPLE_PLAYER_USE_MEEPLE_PENALTY 0

TileMove SimplePlayer::getTileMove(int player, const Tile * tile, const MoveHistoryEntry & move, const TileMovesType & possible)
{
	static constexpr int dx[8] = {-1,  0, +1,  0, -1, +1, +1, -1};
	static constexpr int dy[8] = { 0, -1,  0, +1, -1, -1, +1, +1};
	static constexpr Tile::Side dir[4]    = {Tile::left,  Tile::up,    Tile::right, Tile::down};
	static constexpr Tile::Side oppDir[4] = {Tile::right, Tile::down,  Tile::left,  Tile::up  };
#if SIMPLE_PLAYER_USE_MEEPLE_PENALTY
	static constexpr int meeplePenalties[MEEPLE_COUNT+1] = {5, 4, 3, 2, 1, 1, 0, 0};
	int const meeplePenalty = meeplePenalties[meepleCount];
#else
	static constexpr int meeplePenalty = 0;
#endif

	int const myBonus = game->getPlayerCount();
	int const opponentBonus = myBonus - 1;
	int const meepleCount = game->getPlayerMeeples(player);
	bool const hasMeeples = (meepleCount > 0);

	meepleMoveSet = false;
	if (tile->getCloisterNode() != 0)
	{
		if (hasMeeples)
		{
#if SIMPLE_PLAYER_RULE_CLOISTER_1
			//TODO filter out rotations?
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
#if SIMPLE_PLAYER_RULE_CLOISTER_2
			VarLengthArrayWrapper<TileMove const *, TILE_ARRAY_LENGTH>::type goodMoves;
			int best = -10;
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
#if  SIMPLE_PLAYER_RULE_ROAD_CITY
		VarLengthArrayWrapper<Move, TILE_ARRAY_LENGTH * NODE_ARRAY_LENGTH>::type goodMoves;
		int best = std::numeric_limits<int>::min();
		for (TileMove const & tileMove : possible)
		{
			int tilePoints = 0;
			VarLengthArrayWrapper<MeepleMove, TILE_ARRAY_LENGTH>::type meepleMoves(1);
			int bestNodePoints = 0; //std::numeric_limits<int>::min();
			for (int i = 0; i < 4; ++i)
			{
				Tile const * t = board->getTile(tileMove.x + dx[i], tileMove.y + dy[i]);
				if (t == 0)
					continue;
				int points = 0;

				Tile::Side const & oppDirection = oppDir[i];
				TerrainType const & type = t->getEdge(oppDirection);
				if (type == Field)	//TODO Fields
					continue;
				//else Cities and Roads

				Tile::Side const & direction = dir[i];
				Q_ASSERT(type == tile->getEdge(direction, tileMove.orientation));

				uchar nodeIndex;
				Node const * nn = tile->getFeatureNodeAndIndex(direction, nodeIndex, tileMove.orientation);
				Node const * on = t->getFeatureNode(oppDirection);
				Q_ASSERT(nn != 0);
				Q_ASSERT(on != 0);
				Q_ASSERT(nodeIndex != uchar(-1));
				Q_ASSERT(on->getTerrain() == nn->getTerrain());

				int const score = nn->getScore() + on->getScore();
				uchar const max = on->getMaxMeeples();
				if (max != 0) //isOccupied
				{
					for (uint p = 0; p < game->getPlayerCount(); ++p)
					{
						if (on->getPlayerMeeples(p) != max)
							continue;
						if (p == (uint)player)
							points += myBonus*score;
						else
							points -= opponentBonus * score;
					}
				}
				else if (hasMeeples)
				{
					int nodePoints = myBonus*score - meeplePenalty;
					switch (type)
					{
						case City:
							nodePoints *= 3;
							break;
						case Road:
							nodePoints *= 2;
							break;
						case Field:
						case Cloister:
						case None:
							Q_UNREACHABLE();
							break;
					}

					if (nodePoints > bestNodePoints)
					{
						bestNodePoints = nodePoints;
						meepleMoves.clear();
						meepleMoves.push_back(MeepleMove{nodeIndex});
					}
					else if (nodePoints == bestNodePoints)
					{
						meepleMoves.push_back(MeepleMove{nodeIndex});
					}
				}

				switch (type)
				{
					case City:
						points *= 3;
						break;
					case Road:
						points *= 2;
						break;
					case Field:
					case Cloister:
					case None:
						Q_UNREACHABLE();
						break;
				}
				tilePoints += points;
			}

			tilePoints += bestNodePoints;
			if (tilePoints > best)
			{
				best = tilePoints;
				goodMoves.clear();
				for (MeepleMove const & mm : meepleMoves)
					goodMoves.push_back(Move{tileMove, mm});
			}
			else if (tilePoints == best)
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

MeepleMove SimplePlayer::getMeepleMove(int player, const Tile * tile, const MoveHistoryEntry & move, const MeepleMovesType & possible)
{
	if (meepleMoveSet && std::find(possible.cbegin(), possible.cend(), meepleMove) != possible.cend())
		return meepleMove;
	return RandomPlayer::instance.getMeepleMove(player, tile, move, possible);
}

void SimplePlayer::endGame()
{
}

QString SimplePlayer::getTypeName() const
{
	return "SimplePlayer";
}

Player *SimplePlayer::clone() const
{
	return new SimplePlayer();
}
