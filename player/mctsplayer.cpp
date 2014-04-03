#include "mctsplayer.h"
#include "randomplayer.h"
#include <QElapsedTimer>

MCTSPlayer::MCTSNode::MCTSNode(uchar player, Type type, int size, int playerCount, MCTSNode * parent)
    : player(player),
      type(type),
      notExpanded(size),
      rewards(playerCount),
      parent(parent)
{
	Q_ASSERT(size != 0);
	for (int i = 0; i < playerCount; ++i)
		rewards[i] = 0;
	children.resize(size);
	children.shrink_to_fit();
}

MCTSPlayer::MCTSTileNode::MCTSTileNode(uchar player, TileMovesType && possible, int playerCount, MCTSNode * parent, int parentAction)
    : MCTSNode(player, TypeTile, possible.size(), playerCount, parent),
      possible(possible),
      parentAction(parentAction)
{
}

MCTSPlayer::MCTSMeepleNode::MCTSMeepleNode(uchar player, MeepleMovesType && possible, int playerCount, MCTSNode * parent, TileMove * parentAction)
    : MCTSNode(player, TypeMeeple, possible.size(), playerCount, parent),
      possible(possible),
      parentAction(parentAction)
{
}

MCTSPlayer::MCTSChanceNode::MCTSChanceNode(uchar player, TileCountType const & tileCounts, int playerCount, MCTSNode * parent, MeepleMove * parentAction)
    : MCTSNode(player, TypeChance, tileCounts.size(), playerCount, parent),
      tileCounts(tileCounts),
      parentAction(parentAction)
{
	for (int c : tileCounts)
		if (c == 0)
			--notExpanded;
}


MCTSPlayer::MCTSPlayer(jcz::TileFactory * tileFactory)
    : tileFactory(tileFactory)
{
}

template<>
void MCTSPlayer::apply<int>(int action, Game & g)
{
	g.simPartStepChance(g.getTileIndexByType(action));
}

template<>
void MCTSPlayer::apply<TileMove *>(TileMove * action, Game & g)
{
	g.simPartStepTile(*action);
}

template<>
void MCTSPlayer::apply<MeepleMove *>(MeepleMove * action, Game & g)
{
	g.simPartStepMeeple(*action);
}

template<>
void MCTSPlayer::apply<MCTSPlayer::MCTSTileNode *>(MCTSPlayer::MCTSTileNode * node, Game & g)
{
	if (print)
		qDebug() << "     apply TileNode  " << node->parentAction;
	apply(node->parentAction, g);
}

template<>
void MCTSPlayer::apply<MCTSPlayer::MCTSMeepleNode *>(MCTSPlayer::MCTSMeepleNode * node, Game & g)
{
	if (print)
		qDebug() << "     apply MeepleNode" << node->parentAction;
	apply(node->parentAction, g);
}

template<>
void MCTSPlayer::apply<MCTSPlayer::MCTSChanceNode *>(MCTSPlayer::MCTSChanceNode * node, Game & g)
{
	if (print)
		qDebug() << "     apply ChanceNode" << node->parentAction;
	apply(node->parentAction, g);
}

template<>
void MCTSPlayer::apply<MCTSPlayer::MCTSNode *>(MCTSPlayer::MCTSNode * node, Game & g)
{
	switch (node->type)
	{
		case MCTSNode::TypeTile:
			return apply(static_cast<MCTSTileNode *>(node), g);
		case MCTSNode::TypeMeeple:
			return apply(static_cast<MCTSMeepleNode *>(node), g);
		case MCTSNode::TypeChance:
			return apply(static_cast<MCTSChanceNode *>(node), g);
	}
}

template<>
void MCTSPlayer::unapply<MCTSPlayer::MCTSTileNode>(MCTSPlayer::MCTSTileNode * node, Game & g)
{
	if (print)
		qDebug() << "   unapply TileNode" << node->parentAction;
	g.simPartUndoChance();
}

template<>
void MCTSPlayer::unapply<MCTSPlayer::MCTSMeepleNode>(MCTSPlayer::MCTSMeepleNode * node, Game & g)
{
	if (print)
		qDebug() << "   unapply MeepleNode" << node->parentAction;
	g.simPartUndoTile();
}

template<>
void MCTSPlayer::unapply<MCTSPlayer::MCTSChanceNode>(MCTSPlayer::MCTSChanceNode * node, Game & g)
{
	if (print)
		qDebug() << "   unapply ChanceNode" << node->parentAction;
	g.simPartUndoMeeple();
}

template<>
void MCTSPlayer::unapply<MCTSPlayer::MCTSNode>(MCTSPlayer::MCTSNode * node, Game & g)
{
	switch (node->type)
	{
		case MCTSNode::TypeTile:
			return unapply(static_cast<MCTSTileNode *>(node), g);
		case MCTSNode::TypeMeeple:
			return unapply(static_cast<MCTSMeepleNode *>(node), g);
		case MCTSNode::TypeChance:
			return unapply(static_cast<MCTSChanceNode *>(node), g);
	}
}

void MCTSPlayer::newGame(int /*player*/, const Game * g)
{
	game = g;
	simGame.clearPlayers();
	for (uint i = 0; i < g->getPlayerCount(); ++i)
		simGame.addPlayer(&RandomPlayer::instance);
	simGame.newGame(game->getTileSets(), tileFactory, g->getMoveHistory());
}

void MCTSPlayer::playerMoved(int /*player*/, const Tile * /*tile*/, const MoveHistoryEntry & /*move*/)
{
	auto const & history = game->getMoveHistory();
	for (uint i = simGame.getMoveHistory().size(); i < history.size(); ++i)
		simGame.simStep(history[i]);
}

TileMove MCTSPlayer::getTileMove(int player, const Tile * tile, const MoveHistoryEntry & /*move*/, const TileMovesType & /*placements*/)
{
	//TODO use provided placements

	Q_ASSERT(game->equals(simGame));
	MCTSTileNode * v0 = generateTileNode(0, player, tile->tileType, simGame);

	QElapsedTimer t;
	t.start();
//	for (int i = 0; t.elapsed() < 10*1000; ++i)
	for (int i = 0; i < 600; ++i)
	{
		qDebug() << i;
		Q_ASSERT(game->equals(simGame));
//		if (i == 327)
//			print = true;

		if (print)
			qDebug("Initial apply");

		auto vl = treePolicy(v0);
		auto delta = defaultPolicy(vl);
		backup(vl, delta);
		Q_ASSERT(game->equals(simGame));
	}
	unapply(v0, simGame);

	int b = bestChild0(v0);
	auto a = v0->possible[b];

	MCTSMeepleNode * meepleNode = (*v0->castChildren())[b];
	b = bestChild0(meepleNode);
	meepleMove = meepleNode->possible[b];

	delete v0; //TODO keep and reuse.
	return a;
}

MeepleMove MCTSPlayer::getMeepleMove(int /*player*/, const Tile * /*tile*/, const MoveHistoryEntry & /*move*/, const MeepleMovesType & /*possible*/)
{
	return meepleMove;
}

void MCTSPlayer::endGame()
{
}

const char *MCTSPlayer::getTypeName()
{
	return "MCTSPlayer";
}

MCTSPlayer::MCTSNode * MCTSPlayer::treePolicy(MCTSNode * v)
{
	while (!simGame.isFinished())
	{
		if (v->notExpanded)
		{
			if (print)
				qDebug("Tree policy: not expanded");
			auto r = expand(v);
			return r;
		}
		else
		{
			if (print)
				qDebug("Tree policy: best child");
			v = bestChild(v);
			apply(v, simGame);
		}
	}
	Q_ASSERT(v != 0);
	return v;
}

MCTSPlayer::MCTSNode * MCTSPlayer::expand(MCTSPlayer::MCTSNode * v)
{
	if (print)
		qDebug("Expand");
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
//		while (tileIndex > 0 && tiles[tileIndex-1]->tileType == t->tileType)
//			--tileIndex;
//		t = tiles[tileIndex];

//		vPrime = generateTileNode(v, v->player, t, simGame);
		vPrime = generateTileNode(v, v->player, a, simGame);
	}
	else
	{
		do
		{
			a = r.nextInt(v->children.size());
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
				vPrime = generateMeepleNode(v, v->player, &tn->possible[a], simGame.getTileByType(tn->parentAction), simGame);
				break;
			}
			case MCTSNode::TypeMeeple:
			{
				MCTSMeepleNode * mn = static_cast<MCTSMeepleNode *>(v);
				vPrime = generateChanceNode(v, nextPlayer(v->player), &mn->possible[a], simGame);
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

MCTSPlayer::MCTSNode * MCTSPlayer::bestChild(MCTSNode * v)
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
			qreal val = (Q(vPrime) / qreal(N(vPrime))) + Cp * mySqrt( myLn( N(v) ) / N(vPrime) );
			if (val > max)
			{
				max = val;
				best = vPrime;
			}
		}
	}
	Q_ASSERT(best != 0);
	return best;
}

int MCTSPlayer::bestChild0(MCTSPlayer::MCTSNode * v)
{
	qreal max = -std::numeric_limits<qreal>::infinity();
	int a = -1;
	for (int i = 0, s = v->children.size(); i < s; ++i)
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

MCTSPlayer::RewardType MCTSPlayer::defaultPolicy(MCTSNode * v)
{
	if (print)
		qDebug("Default policy...");
	switch (v->type)
	{
		case MCTSNode::TypeTile:
		{
			Tile const * tile = simGame.getTiles()[simGame.simEntry.tile];
			auto && possible = simGame.getPossibleTilePlacements(tile);
			if (possible.isEmpty())
				simGame.simPartStepTile(TileMove());
			else
				simGame.simPartStepTile(RandomPlayer::instance.getTileMove(v->player, tile, simGame.simEntry, possible));
			// no break
		}
		case MCTSNode::TypeMeeple:
		{
			Tile const * tile = simGame.getTiles()[simGame.simEntry.tile];
			simGame.simPartStepMeeple(RandomPlayer::instance.getMeepleMove(v->player, tile, simGame.simEntry, simGame.getPossibleMeeplePlacements(tile)));
		}
		case MCTSNode::TypeChance:
			break;
	}

	int steps = 0;
	do
		++steps;
	while (simGame.simStep(&RandomPlayer::instance));

	RewardType && reward = utilities(simGame.getScores(), simGame.getPlayerCount());

	for (int i = 0; i < steps; ++i)
		simGame.undo();

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

	if (print)
		qDebug() << "Default policy end";

	return reward;
}

void MCTSPlayer::backup(MCTSPlayer::MCTSNode * v, RewardType const & delta)
{
	if (print)
		qDebug("Back up");
	while (true)
	{
		++N(v);

		// Not sure about this:
		for (int i = 0; i < delta.size(); ++i)
			v->rewards[i] += delta[i];

		if (v->parent == 0)
			break;
		unapply(v, simGame);
		v = v->parent;
	}
}

MCTSPlayer::MCTSTileNode * MCTSPlayer::generateTileNode(MCTSNode * parent, uchar player, int parentAction, Game & g)
{
	Tile const * t = g.getTileByType(parentAction);
	apply(parentAction, g);
	TileMovesType && possible = g.getPossibleTilePlacements(t);
	if (possible.size() == 0)
		possible.append(TileMove()); // I could probably add a null MeepleMove child here, too.
	MCTSTileNode * node = new MCTSTileNode(player, std::move(possible), g.getPlayerCount(), parent, parentAction);
	assertRewards(node);
	return node;
}

MCTSPlayer::MCTSMeepleNode * MCTSPlayer::generateMeepleNode(MCTSNode * parent, uchar player, TileMove * parentAction, const Tile * t, Game & g)
{
	apply(parentAction, g);
	MCTSMeepleNode * node;
	{
		MeepleMovesType possible;
		if (g.getPlayerMeeples(player) == 0 || parentAction->isNull())
			possible.push_back(MeepleMove());
		else
			possible = g.getPossibleMeeplePlacements(t);
		node = new MCTSMeepleNode(player, std::move(possible), g.getPlayerCount(), parent, parentAction);
	}
	assertRewards(node);
	return node;
}

MCTSPlayer::MCTSChanceNode *MCTSPlayer::generateChanceNode(MCTSNode * parent, uchar player, MeepleMove * parentAction, Game & g)
{
	apply(parentAction, g);
	TileCountType const & tileCounts = g.getTileCounts();
	MCTSChanceNode * node = new MCTSChanceNode(player, tileCounts, g.getPlayerCount(), parent, parentAction);
	assertRewards(node);

	return node;
}

void MCTSPlayer::assertRewards(MCTSPlayer::MCTSNode * n)
{
	Q_UNUSED(n);
	Q_ASSERT(n != 0);
	Q_ASSERT((uint)n->rewards.size() == game->getPlayerCount());
	for (uint i = 0; i < game->getPlayerCount(); ++i)
		Q_ASSERT(n->rewards[i] == 0);
}
