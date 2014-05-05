#include "mctsplayer.h"
#include "randomplayer.h"
#include <QElapsedTimer>

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
	for (int c : tileCounts)
		if (c == 0)
			--MCTSChanceNode::notExpanded;
}


MCTS_T
constexpr MCTSPlayer MCTS_TU::MCTSPlayer(jcz::TileFactory * tileFactory, int m, bool mIsTimeout)
	: tileFactory(tileFactory),
	  typeName(QString("MCTSPlayer<%1, %2>").arg(UtilityProvider::name).arg(Playout::name)),
	  M(m),
	  useTimeout(mIsTimeout)
{
}

MCTS_T
void MCTSPlayer MCTS_TU::playerMoved(int /*player*/, const Tile * /*tile*/, const MoveHistoryEntry & /*move*/)
{
	syncGame();
}

MCTS_T
TileMove MCTSPlayer MCTS_TU::getTileMove(int player, const Tile * tile, const MoveHistoryEntry & /*move*/, const TileMovesType & /*placements*/)
{
	syncGame();

	//TODO use provided placements

	Q_ASSERT(game->equals(simGame));
	Q_ASSERT(simGame.getNextPlayer() == player);
	Q_UNUSED(player);
	MCTSTileNode * v0 = generateTileNode(0, tile->tileType, simGame);

	{
		QElapsedTimer t;
		if (useTimeout)
			t.start();
		int i = 0;
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
		}
		while (useTimeout ? !t.hasExpired(M) : i < M);
	}
	unapplyNode(v0, simGame);

	int b = bestChild0(v0);
	auto a = v0->possible[b];

	MCTSMeepleNode * meepleNode = (*v0->castChildren())[b];
	b = bestChild0(meepleNode);
	meepleMove = meepleNode->possible[b];

	delete v0; //TODO keep and reuse.
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
}

MCTS_T
QString MCTSPlayer MCTS_TU::getTypeName()
{
	return typeName;
}

MCTS_T
Player * MCTSPlayer MCTS_TU::clone() const
{
	return new MCTSPlayer(tileFactory, M, useTimeout);
}

MCTS_T
typename MCTSPlayer MCTS_TU::MCTSNode * MCTSPlayer MCTS_TU::treePolicy(MCTSNode * v)
{
	while (!simGame.isFinished())
	{
		if (v->notExpanded)
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
	Q_ASSERT(v != 0);
	return v;
}

MCTS_T
typename MCTSPlayer MCTS_TU::MCTSNode * MCTSPlayer MCTS_TU::expand(MCTSNode * v)
{
	//TODO maybe store untried nodes somehow?
	int a;
	MCTSNode * vPrime;
	if (v->type == MCTSNode::TypeChance)
	{
		Tile const * t;
		auto const & tiles = simGame.getTiles();
		int tileIndex;
		do
		{
			tileIndex = r.nextInt(tiles.size());
			t = tiles[tileIndex];
			a = t->tileType;
			vPrime = v->children[a];

#if MCTS_COUNT_EXPAND_HITS
			++miss;
#endif
		} while (vPrime != 0);

		vPrime = generateTileNode(v, a, simGame);
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
		switch (v->type)
		{
			case MCTSNode::TypeTile:
			{
				MCTSTileNode * tn = static_cast<MCTSTileNode *>(v);
				vPrime = generateMeepleNode(v, &tn->possible[a], simGame.getTileByType(tn->parentAction), simGame);
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
typename MCTSPlayer MCTS_TU::MCTSNode * MCTSPlayer MCTS_TU::bestChild(MCTSNode * v)
{
	MCTSNode * best = 0;
	if (v->type == MCTSNode::TypeChance)
	{
		QList<Tile *> const & tiles = simGame.getTiles();
		best = v->children[tiles[r.nextInt(tiles.size())]->tileType];
	}
	else
	{
		qreal max = -std::numeric_limits<qreal>::infinity();
		for (auto * vPrime : v->children)
		{
			if (vPrime == 0)
				continue;
			qreal val = (qreal(Q(vPrime)) / qreal(N(vPrime))) + Cp * (MCTSPlayer MCTS_TU::math).sqrt( math.ln( N(v) ) / N(vPrime) );
			if (val > max)
			{
				max = val;
				best = vPrime;
			}
		}
	}
	Q_ASSERT(best != 0);
	if ( Q_UNLIKELY(best == 0) )	//This should not happen. Used this for debugging, lets just keep it in case something goes wrong.
	{
		best = v->children[0];
		int level = 0;
		for (MCTSNode * n = v; n->parent != 0; n = n->parent)
			++level;
		qWarning() << getTypeName() << "::bestChild: best == 0";
		qWarning() << "\tlevel:" << level << "  children:" << v->children.size() << "  N:" << N(v);
		for (auto * vPrime : v->children)
			qWarning() << "\tchild: " << vPrime << "  Q:" << Q(vPrime) << "  N:" << N(vPrime) << "  value:" << ((qreal(Q(vPrime)) / qreal(N(vPrime))) + Cp * (MCTSPlayer MCTS_TU::math).sqrt( math.ln( N(v) ) / N(vPrime) ));
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
		qreal val = Q(vPrime) / qreal(N(vPrime));
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

	RewardListType && reward = utilities(simGame.getScores(), simGame.getPlayerCount());

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

		if (v->parent == 0)
			break;
		unapplyNode(v, simGame);
		v = v->parent;
	}
}

MCTS_T
void MCTSPlayer MCTS_TU::syncGame()
{
	Util::syncGamesFast(*game, simGame);
}

MCTS_T
typename MCTSPlayer MCTS_TU::MCTSTileNode * MCTSPlayer MCTS_TU::generateTileNode(MCTSNode * parent, int parentAction, Game & g)
{
	int player = g.getNextPlayer();
	Tile const * t = g.getTileByType(parentAction);
	applyChance(parentAction, g);
	TileMovesType && possible = g.getPossibleTilePlacements(t);
	if (possible.size() == 0)
		possible.push_back(TileMove()); // I could probably add a null MeepleMove child here, too.
	MCTSTileNode * node = new MCTSTileNode((uchar)player, std::move(possible), parent, parentAction);
	return node;
}

MCTS_T
typename MCTSPlayer MCTS_TU::MCTSMeepleNode * MCTSPlayer MCTS_TU::generateMeepleNode(MCTSNode * parent, TileMove * parentAction, const Tile * t, Game & g)
{
	int player = g.getNextPlayer();
	applyTile(parentAction, g);
	MCTSMeepleNode * node;
	{
		MeepleMovesType && possible = getPossibleMeeples(player, parentAction, t, g);
		node = new MCTSMeepleNode((uchar)player, std::move(possible), parent, parentAction);
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
typename MCTSPlayer MCTS_TU::RewardListType MCTSPlayer MCTS_TU::utilities(const int * scores, const int playerCount)
{
	return utilityProvider.utilities(scores, playerCount);
}

MCTS_T
void MCTSPlayer MCTS_TU::newGame(int player, Game const * g)
{
	game = g;
	simGame.clearPlayers();
	for (uint i = 0; i < g->getPlayerCount(); ++i)
		simGame.addPlayer(&RandomPlayer::instance);
	simGame.newGame(g->getTileSets(), tileFactory, g->getMoveHistory());
	utilityProvider.newGame(player, g);
}
