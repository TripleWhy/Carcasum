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

#include "mctsplayer.h"
#include "randomplayer.h"
#include "core/board.h"
#include "core/util.h"

#define MCTS_T template<class UtilityProvider, class Playout>
#define MCTS_TU <UtilityProvider, Playout>

MCTS_T
Util::Math const & MCTSPlayer MCTS_TU::math = Util::Math::instance;

MCTS_T
MCTSPlayer MCTS_TU::MCTSNode::MCTSNode(uchar player, Type type, int size, MCTSNode * parent)
    : player(player),
      type(type),
      notExpanded(size),
      parent(parent)
{
	Q_ASSERT(size != 0);
	children.resize(size);
	children.shrink_to_fit();
}

MCTS_T
MCTSPlayer MCTS_TU::MCTSTileNode::MCTSTileNode(uchar player, TileMovesType && possible, MCTSNode * parent, int parentAction)
    : MCTSNode(player, MCTSNode::TypeTile, possible.size(), parent),
      possible(possible),
      parentAction(parentAction)
{
}

MCTS_T
MCTSPlayer MCTS_TU::MCTSMeepleNode::MCTSMeepleNode(uchar player, MeepleMovesType && possible, MCTSNode * parent, TileMove * parentAction)
    : MCTSNode(player, MCTSNode::TypeMeeple, possible.size(), parent),
      possible(possible),
      parentAction(parentAction)
{
}

MCTS_T
MCTSPlayer MCTS_TU::MCTSChanceNode::MCTSChanceNode(uchar player, TileCountType const & tileCounts, MCTSNode * parent, MeepleMove * parentAction)
    : MCTSNode(player, MCTSNode::TypeChance, tileCounts.size(), parent),
      tileCounts(tileCounts),
      parentAction(parentAction)
{
}


MCTS_T
constexpr MCTSPlayer MCTS_TU::MCTSPlayer(jcz::TileFactory * tileFactory, bool reuseTree, const uint m, const bool mIsTimeout, qreal const Cp, bool nodePriors, bool progressiveWidening, bool progressiveBias)
    : tileFactory(tileFactory),
      typeName(QString("MCTSPlayer<%1, %2>(reuseTree=%3, m=%4, mIsTimeout=%5, Cp=%6, nodePriors=%7, progressiveWidening=%8, progressiveBias=%9)").arg(UtilityProvider::name).arg(playoutPolicy.name).arg(reuseTree).arg(m).arg(mIsTimeout).arg(Cp).arg(nodePriors).arg(progressiveWidening).arg(progressiveBias)),
      M(m),
      useTimeout(mIsTimeout),
      Cp(Cp),
      reuseTree(reuseTree),
      nodePriors(nodePriors),
      progressiveWidening(progressiveWidening),
      progressiveBias(progressiveBias)
{
}

MCTS_T
void MCTSPlayer MCTS_TU::playerMoved(int /*player*/, const Tile * /*tile*/, const MoveHistoryEntry & /*move*/)
{
	syncGame();
}

MCTS_T
void MCTSPlayer MCTS_TU::undoneMove(const MoveHistoryEntry & /*move*/)
{
	fullSyncGame();
}

MCTS_T
TileMove MCTSPlayer MCTS_TU::getTileMove(int player, const Tile * tile, const MoveHistoryEntry & /*move*/, const TileMovesType & /*placements*/)
{
	Util::ExpireTimer t;
	if (useTimeout)
		t.start();

	syncGame();

	//TODO use provided placements

	Q_ASSERT(game->equals(simGame));
	Q_ASSERT(simGame.getNextPlayer() == player);
	Q_UNUSED(player);

	MCTSTileNode * v0;
	if (!reuseTree)
		v0 = generateTileNode(0, tile->tileType, simGame);
	else
	{
		v0 = 0;
		if (rootNode != 0)
			v0 = (*rootNode->castChildren())[tile->tileType];

		if (v0 != 0)
			applyNode(v0, simGame);
		else
		{
			rootNode = new MCTSChanceNode((uchar)player, simGame.getTileCounts(), 0, 0);

			v0 = generateTileNode(rootNode, tile->tileType, simGame);
			rootNode->children[tile->tileType] = v0;
		}
	}

	{
		uint i = 0;
		do
		{
//			qDebug() << i;
			Q_ASSERT(game->equals(simGame));

			auto vl = treePolicy(v0);
			auto const & delta = defaultPolicy(vl);
			backup(vl, delta);
			Q_ASSERT(game->equals(simGame));
			if (!useTimeout)
				++i;
		} while (useTimeout ? !t.hasExpired(M) : i < M);
	}
	unapplyNode(v0, simGame);

	int b = bestChild0(v0);
	auto a = v0->possible[b];

	MCTSMeepleNode * meepleNode = (*v0->castChildren())[b];
	b = bestChild0(meepleNode);
	meepleMove = meepleNode->possible[b];

	if (!reuseTree)
	{
		delete v0;
//		int depth = v0->deleteCounting(0);
//		qDebug() << "depth:" << depth;
	}

	return a;
}

MCTS_T
MeepleMove MCTSPlayer MCTS_TU::getMeepleMove(int /*player*/, const Tile * /*tile*/, const MoveHistoryEntry & /*move*/, const MeepleMovesType & /*possible*/)
{
	return meepleMove;
}

MCTS_T
void MCTSPlayer MCTS_TU::endGame()
{
	if (reuseTree)
	{
		delete rootNode;
		rootNode = 0;
	}
}

MCTS_T
QString MCTSPlayer MCTS_TU::getTypeName() const
{
	return typeName;
}

MCTS_T
Player * MCTSPlayer MCTS_TU::clone() const
{
	return new MCTSPlayer(tileFactory, reuseTree, M, useTimeout, Cp, nodePriors, progressiveWidening, progressiveBias);
}

MCTS_T
typename MCTSPlayer MCTS_TU::MCTSNode * MCTSPlayer MCTS_TU::treePolicy(MCTSNode * v)
{
	while (!simGame.isFinished())
	{
		if (v->type == MCTSNode::TypeChance)
		{
			auto const & tiles = simGame.getTiles();
			int tileIndex = r.nextInt(tiles.size());
			Tile const * t = tiles[tileIndex];
			int const a = t->tileType;
			if (v->children[a] == 0)
			{
				auto r = expandChance(v, a);
				return r;
			}
			else
			{
				v = v->children[a];
				applyNode(v, simGame);
			}
		}
		else
		{
			if (expansionCandidate(v))
			{
				auto r = expand(v);
				return r;
			}
			else
			{
				v = bestChild(v);
				applyNode(v, simGame);
			}
		}
	}
	Q_ASSERT(v != 0);
	return v;
}

MCTS_T
typename MCTSPlayer MCTS_TU::MCTSNode * MCTSPlayer MCTS_TU::expand(MCTSNode * v)
{
	Q_ASSERT(v->type != MCTSNode::TypeChance);

	//TODO maybe store untried nodes somehow?
	int a;
	MCTSNode * vPrime;
	if (progressiveWidening)
	{
		a = (int)(v->children.size() - v->notExpanded);
		vPrime = v->children[a];
	}
	else
	{
		do
		{
			a = r.nextInt((int)v->children.size());
			vPrime = v->children[a];
#if MCTS_COUNT_EXPAND_HITS
			++miss;
#endif
		} while (vPrime != 0);
	}
	switch (v->type)
	{
		case MCTSNode::TypeTile:
		{
			MCTSTileNode * tn = static_cast<MCTSTileNode *>(v);
			vPrime = generateMeepleNode(v, &tn->possible[a], simGame.getTileByType(tn->parentAction), simGame, a);
			break;
		}
		case MCTSNode::TypeMeeple:
		{
			MCTSMeepleNode * mn = static_cast<MCTSMeepleNode *>(v);
			vPrime = generateChanceNode(v, &mn->possible[a], simGame);
			break;
		}
		case MCTSNode::TypeChance:
			Q_UNREACHABLE();
			break;
	}

#if MCTS_COUNT_EXPAND_HITS
		--miss;
		++hit;
#endif

	v->children[a] = vPrime;
	--v->notExpanded;
	return vPrime;
}

MCTS_T
typename MCTSPlayer MCTS_TU::MCTSTileNode * MCTSPlayer MCTS_TU::expandChance(MCTSNode * v, int a)
{
	MCTSTileNode * vPrime = generateTileNode(v, a, simGame);
	v->children[a] = vPrime;
	--v->notExpanded;
	return vPrime;
}

MCTS_T
typename MCTSPlayer MCTS_TU::MCTSNode * MCTSPlayer MCTS_TU::bestChild(MCTSNode * v)
{
	Q_ASSERT(v->type != MCTSNode::TypeChance);

	MCTSNode * best = 0;
	qreal max = -std::numeric_limits<qreal>::infinity();

#if ASSERT_ENABLED
	uint childNSum = 0;
#endif
	for (auto * vPrime : v->children)
	{
		if (progressiveWidening)
		{
			if (vPrime == 0)
				break;
		}
		else
			Q_ASSERT(vPrime != 0);

#if ASSERT_ENABLED
		childNSum += N(vPrime);
#endif
		qreal val = UCB(v, vPrime);
		if (val > max)
		{
			max = val;
			best = vPrime;
		}
	}
	Q_ASSERT((v->parent == rootNode && NParent(v) == childNSum) || (v->parent != rootNode && NParent(v) == childNSum + 1));

	Q_ASSERT(best != 0);
	if ( Q_UNLIKELY(best == 0) )	//This should not happen. Used this for debugging, lets just keep it in case something goes wrong.
	{
		best = v->children[0];
		int level = 0;
		for (MCTSNode * n = v; n->parent != 0; n = n->parent)
			++level;
		qWarning().nospace() << getTypeName() << "::bestChild: best == 0";
		qWarning() << "\tlevel:" << level << "  children:" << v->children.size() << "  NParent:" << NParent(v);
		for (auto * vPrime : v->children)
			qWarning() << "\tchild: " << vPrime << "  Q:" << Q(vPrime) << "  N:" << N(vPrime) << "  value:" << ((qreal(Q(vPrime)) / qreal(N(vPrime))) + Cp * (MCTSPlayer MCTS_TU::math).sqrt( math.ln( NParent(v) ) / N(vPrime) ));
	}
	return best;
}

MCTS_T
int MCTSPlayer MCTS_TU::bestChild0(MCTSNode * v)
{
#if MCTS_PRINT_UTILITIES
	QList<MCTSNode *> queue;
	queue.append(v);

	qDebug("MCTSPlayer::bestChild0 (showing only 100 nodes)");
	for (int i = 0; !queue.isEmpty() && i < 100; ++i)
	{
		MCTSNode * n = queue.takeFirst();
		qDebug() << n << "parent:" << n->parent;
		qDebug() << "  type:" << n->type << "player:" << n->player << "notExpanded:" << n->notExpanded << "visitCount:" << n->visitCount << "reward:" << n->reward;
		qDebug() << "  Q/N:" << (Q(n) / qreal(N(n)));
		qDebug() << "  children:";
		for (MCTSNode * c : n->children)
		{
			qDebug() << "   " << c;
			if (c != 0)
				queue.append(c);
		}
	}
	qDebug();
	qDebug();
#endif

	qreal max = -std::numeric_limits<qreal>::infinity();
	int a = 0;
	for (int i = 0, s = (int)v->children.size(); i < s; ++i)
	{
		auto * vPrime = v->children[i];
		if (vPrime == 0)
			continue;
		qreal val = qreal(Q(vPrime)) / qreal(N(vPrime));
		if (val > max)
		{
			max = val;
			a = i;
		}
	}
	return a;
}

MCTS_T
typename MCTSPlayer MCTS_TU::RewardListType MCTSPlayer MCTS_TU::defaultPolicy(MCTSNode * v)
{
	switch (v->type)
	{
		case MCTSNode::TypeTile:
		{
			Tile const * tile = simGame.simTile;
			TileMove tileMove;
			TileMovesType && possibleTiles = simGame.getPossibleTilePlacements(tile);
			if (possibleTiles.size())
				tileMove = playoutPolicy.chooseTileMove(v->player, tile, simGame.simEntry, possibleTiles);
			simGame.simPartStepTile(tileMove);

			MeepleMovesType && possibleMeeples = getPossibleMeeples(v->player, &tileMove, tile, simGame);
			MeepleMove const & meepleMove = playoutPolicy.chooseMeepleMove(v->player, tile, simGame.simEntry, possibleMeeples);
			simGame.simPartStepMeeple(meepleMove);

			break;
		}
		case MCTSNode::TypeMeeple:
		{
			MeepleMove const & meepleMove = playoutPolicy.chooseMeepleMove(v->player, 0, simGame.simEntry, static_cast<MCTSMeepleNode *>(v)->possible);
			simGame.simPartStepMeeple(meepleMove);
			break;
		}
		case MCTSNode::TypeChance:
			break;
	}

	int steps = playoutPolicy.playout(simGame);
#if COUNT_PLAYOUTS
	++playouts;
#endif

	RewardListType && reward = utilities(simGame.getScores(), simGame.getPlayerCount(), &simGame);

	playoutPolicy.undoPlayout(simGame, steps);

	switch (v->type)
	{
		case MCTSNode::TypeTile:
			simGame.simPartUndoMeeple();
			simGame.simPartUndoTile();
			break;
		case MCTSNode::TypeMeeple:
			simGame.simPartUndoMeeple();
			break;
		case MCTSNode::TypeChance:
			break;
	}

	return reward;
}

MCTS_T
void MCTSPlayer MCTS_TU::backup(MCTSNode * v, RewardListType const & delta)
{
	while (true)
	{
		++N(v);
		Q(v) += delta[v->player];
		Q_ASSERT((v->parent == rootNode && NParent(v) == v->childNSum()) || (v->parent != rootNode && NParent(v) == v->childNSum() + 1) || simGame.isFinished());

		if (v->parent == rootNode)
			break;
		unapplyNode(v, simGame);
		v = v->parent;
	}
}

MCTS_T
void MCTSPlayer MCTS_TU::syncGame()
{
	if (!reuseTree)
		Util::syncGamesFast(*game, simGame);
	else
	{
		auto const & history = game->getMoveHistory();
		for (size_t i = simGame.getMoveHistory().size(), s = history.size(); i < s; ++i)
		{
			MoveHistoryEntry const & e = history[i];
			simGame.simStep(e);

			if (rootNode != 0)
			{
				MCTSChanceNode * cn = 0;

				MCTSTileNode * tn = (*rootNode->castChildren())[ e.tileType ];
				if (tn != 0)
				{
					int mIndex = -1;
					for (int i = 0; i < tn->possible.size(); ++i)
						if (tn->possible[i] == e.move.tileMove)
						{
							mIndex = i;
							break;
						}
					Q_ASSERT(mIndex != -1);
					if (mIndex != -1)
					{
						MCTSMeepleNode * mn = (*tn->castChildren())[mIndex];
						if (mn != 0)
						{
							int cIndex = -1;
							for (int i = 0; i < mn->possible.size(); ++i)
								if (mn->possible[i] == e.move.meepleMove)
								{
									cIndex = i;
									break;
								}
							Q_ASSERT(cIndex != -1);
							if (cIndex != -1)
							{
								cn = (*mn->castChildren())[cIndex];
								if (cn != 0)
									cn->parent = 0;
							}
							else
							{
								qWarning("cIndex == -1");
							}
						}
					}
					else
					{
						qWarning("mIndex == -1");
						qDebug() << "tn->possible.size():" << tn->possible.size();
						for (TileMove const & p : tn->possible)
							qDebug() << p.x << p.y << p.orientation;
						qDebug() << "history entry:";
						qDebug() << e.move.tileMove.x << e.move.tileMove.y << e.move.tileMove.orientation;
						qDebug();
					}
				}

				rootNode->deleteExcept(cn);
				rootNode = cn;
			}
		}
	}
}

MCTS_T
void MCTSPlayer MCTS_TU::fullSyncGame()
{
	Util::syncGames(*game, simGame);
	if (reuseTree)
	{
		delete rootNode;
		rootNode = 0;
	}
}

MCTS_T
typename MCTSPlayer MCTS_TU::MCTSTileNode * MCTSPlayer MCTS_TU::generateTileNode(MCTSNode * parent, int parentAction, Game & g)
{
	int player = g.getNextPlayer();
	Tile const * t = g.getTileByType(parentAction);
	MCTSTileNode * node;
	applyChance(parentAction, g);
	{
		TileMovesType && possible = g.getPossibleTilePlacements(t);
		if (possible.size() == 0)
			possible.push_back(TileMove()); // I could probably add a null MeepleMove child here, too.
		node = new MCTSTileNode((uchar)player, std::move(possible), parent, parentAction);
	}

	if (progressiveWidening && !node->possible[0].isNull())
	{
		//sort possible
		int sum;
		auto const & ratings = SimplePlayer3::rateAllNested(sum, &g, player, g.simTile, node->possible);
		Q_ASSERT(ratings.size() == node->possible.size());;

		std::multimap<int, SimplePlayer3::NestedTileRating const *> map;
		for (auto const & rating : ratings)
		{
			map.insert( {rating.tileRating, &rating} );
		}

		int index = 0;
		for (auto it = map.rbegin(); it != map.rend(); ++it)
		{
			node->possible[index] = it->second->tileMove;
			node->meepleRatings.push_back(std::move(it->second->meepleRatings));
			++index;
		}
	}

	if (progressiveBias)
	{
		node->heuristicValue = utilityProvider.utility(g.getScores(), g.getPlayerCount(), player, &g);
	}
	if (nodePriors)
	{
		N(node) += nodePriorsInitiatPlayouts;
		if (progressiveBias)
			Q(node) += nodePriorsInitiatPlayouts * node->heuristicValue;
		else
			Q(node) += nodePriorsInitiatPlayouts * utilityProvider.utility(g.getScores(), g.getPlayerCount(), player, &g);
	}

	return node;
}

MCTS_T
typename MCTSPlayer MCTS_TU::MCTSMeepleNode * MCTSPlayer MCTS_TU::generateMeepleNode(MCTSNode * parent, TileMove * parentAction, const Tile * t, Game & g, int parentA)
{
	int player = g.getNextPlayer();
	applyTile(parentAction, g);
	MCTSMeepleNode * node;
	{
		MeepleMovesType && possible = getPossibleMeeples(player, parentAction, t, g);
		if (progressiveWidening && possible.size() > 1)
		{
			//sort possible
			SimplePlayer3::RatingsNMeepleType & meepleRatings = static_cast<MCTSTileNode *>(parent)->meepleRatings[parentA];

			std::multimap<int, MeepleMove const *> map;
			for (SimplePlayer3::NestedMeepleRating const & rating : meepleRatings)
			{
#if SIMPLE_PLAYER3_RULE_FIELD
				if (Util::contains(possible, rating.meepleMove))
#endif
					map.insert( {rating.meepleRating, &rating.meepleMove} );
			}
			Q_ASSERT(map.size() == (uint)possible.size());

			int index = 0;
			for (auto it = map.rbegin(); it != map.rend(); ++it)
			{
				possible[index++] = *it->second;
			}
		}
		node = new MCTSMeepleNode((uchar)player, std::move(possible), parent, parentAction);
	}

	if (progressiveBias)
	{
		node->heuristicValue = utilityProvider.utility(g.getScores(), g.getPlayerCount(), player, &g);
	}
	if (nodePriors)
	{
		N(node) += nodePriorsInitiatPlayouts;
		if (progressiveBias)
			Q(node) += nodePriorsInitiatPlayouts * node->heuristicValue;
		else
			Q(node) += nodePriorsInitiatPlayouts * utilityProvider.utility(g.getScores(), g.getPlayerCount(), player, &g);
	}

	return node;
}

MCTS_T
typename MCTSPlayer MCTS_TU::MCTSChanceNode * MCTSPlayer MCTS_TU::generateChanceNode(MCTSNode * parent, MeepleMove * parentAction, Game & g)
{
	int player = g.getNextPlayer();
	applyMeeple(parentAction, g);
	TileCountType const & tileCounts = g.getTileCounts();
	MCTSChanceNode * node = new MCTSChanceNode((uchar)player, tileCounts, parent, parentAction);

	if (progressiveBias)
	{
		node->heuristicValue = utilityProvider.utility(g.getScores(), g.getPlayerCount(), player, &g);
	}
	if (nodePriors)
	{
		N(node) += nodePriorsInitiatPlayouts;
		if (progressiveBias)
			Q(node) += nodePriorsInitiatPlayouts * node->heuristicValue;
		else
			Q(node) += nodePriorsInitiatPlayouts * utilityProvider.utility(g.getScores(), g.getPlayerCount(), player, &g);
	}

	return node;
}

MCTS_T
void MCTSPlayer MCTS_TU::applyChance(int action, Game & g)
{
	g.simPartStepChance(g.getTileIndexByType(action));
}

MCTS_T
void MCTSPlayer MCTS_TU::applyTile(TileMove * action, Game & g)
{
	g.simPartStepTile(*action);
}

MCTS_T
void MCTSPlayer MCTS_TU::applyMeeple(MeepleMove * action, Game & g)
{
	g.simPartStepMeeple(*action);
}

MCTS_T
void MCTSPlayer MCTS_TU::applyNode(MCTSNode * node, Game & g)
{
	switch (node->type)
	{
		case MCTSNode::TypeTile:
			return applyChance(static_cast<MCTSTileNode *>(node)->parentAction, g);
		case MCTSNode::TypeMeeple:
			return applyTile(static_cast<MCTSMeepleNode *>(node)->parentAction, g);
		case MCTSNode::TypeChance:
			return applyMeeple(static_cast<MCTSChanceNode *>(node)->parentAction, g);
	}
}

MCTS_T
void MCTSPlayer MCTS_TU::unapplyChance(Game & g)
{
	g.simPartUndoChance();
}

MCTS_T
void MCTSPlayer MCTS_TU::unapplyTile(Game & g)
{
	g.simPartUndoTile();
}

MCTS_T
void MCTSPlayer MCTS_TU::unapplyMeeple(Game & g)
{
	g.simPartUndoMeeple();
}

MCTS_T
void MCTSPlayer MCTS_TU::unapplyNode(MCTSPlayer::MCTSNode * node, Game & g)
{
	switch (node->type)
	{
		case MCTSNode::TypeTile:
			unapplyChance(g);
			break;
		case MCTSNode::TypeMeeple:
			unapplyTile(g);
			break;
		case MCTSNode::TypeChance:
			unapplyMeeple(g);
			break;
	}
}

MCTS_T
typename MCTSPlayer MCTS_TU::RewardListType MCTSPlayer MCTS_TU::utilities(const int * scores, const int playerCount, Game const * g)
{
	return utilityProvider.utilities(scores, playerCount, g);
}

MCTS_T
void MCTSPlayer MCTS_TU::newGame(int player, Game const * g)
{
	endGame();
	game = g;
	simGame.clearPlayers();
	for (uint i = 0; i < g->getPlayerCount(); ++i)
		simGame.addPlayer(&RandomPlayer::instance);
	simGame.newGame(g->getTileSets(), tileFactory, g->getMoveHistory());
	utilityProvider.newGame(player, g);
	playoutPolicy.newGame(player, g);
}
