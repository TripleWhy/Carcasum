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
}


MCTSPlayer::MCTSPlayer(jcz::TileFactory * tileFactory)
    : tileFactory(tileFactory)
{
}

template<>
void MCTSPlayer::apply<MCTSPlayer::MCTSTileNode>(MCTSPlayer::MCTSTileNode * node, Game & g)
{
	g.simPartStepChance(g.getTileIndexByType(node->parentAction));
}

template<>
void MCTSPlayer::apply<MCTSPlayer::MCTSMeepleNode>(MCTSPlayer::MCTSMeepleNode * node, Game & g)
{
	g.simPartStepTile(*node->parentAction);
}

template<>
void MCTSPlayer::apply<MCTSPlayer::MCTSChanceNode>(MCTSPlayer::MCTSChanceNode * node, Game & g)
{
	g.simPartStepMeeple(*node->parentAction);
}

template<>
void MCTSPlayer::apply<MCTSPlayer::MCTSNode>(MCTSPlayer::MCTSNode * node, Game & g)
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
void MCTSPlayer::unapply<MCTSPlayer::MCTSTileNode>(MCTSPlayer::MCTSTileNode * /*node*/, Game & g)
{
	g.simPartUndoChance();
}

template<>
void MCTSPlayer::unapply<MCTSPlayer::MCTSMeepleNode>(MCTSPlayer::MCTSMeepleNode * /*node*/, Game & g)
{
	g.simPartUndoTile();
}

template<>
void MCTSPlayer::unapply<MCTSPlayer::MCTSChanceNode>(MCTSPlayer::MCTSChanceNode * /*node*/, Game & g)
{
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
	for (int i = 0; t.elapsed() < 10*1000; ++i)
	{
		Q_ASSERT(game->equals(simGame));

		apply(v0, simGame);

		auto vl = treePolicy(v0);
		auto delta = defaultPolicy(vl);
		backup(vl, delta);
		Q_ASSERT(game->equals(simGame));
	}
	int b = bestChild0(v0);
	auto a = v0->possible[b];

	MCTSMeepleNode * meepleNode = v0->castChildren()[b];
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
			auto r = expand(v);
			apply(r, simGame);
			return r;
		}
		else
		{
			v = bestChild(v);
			apply(v, simGame);
		}
	}
	return v;
}

MCTSPlayer::MCTSNode * MCTSPlayer::expand(MCTSPlayer::MCTSNode * v)
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
	qreal max = std::numeric_limits<qreal>::min();
	MCTSNode * best = 0;
	for (auto * vPrime : v->children)
	{
		qreal val = (Q(vPrime) / qreal(N(vPrime))) + Cp * mySqrt( myLn( N(v) ) / N(vPrime) );
		if (val > max)
		{
			max = val;
			best = vPrime;
		}
	}
	return best;
}

int MCTSPlayer::bestChild0(MCTSPlayer::MCTSNode * v)
{
	qreal max = std::numeric_limits<qreal>::min();
	int a;
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
	switch (v->type)
	{
		case MCTSNode::TypeTile:
		{
			Tile const * tile = simGame.getTiles()[simGame.simEntry.tile];
			simGame.simPartStepTile(RandomPlayer::instance.getTileMove(v->player, tile, simGame.simEntry, simGame.getPossibleTilePlacements(tile)));
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

	// Not sure about this:
	RewardType reward(simGame.getPlayerCount());
	for (int i = 0; i < reward.size(); ++i)
		reward[i] = simGame.getPlayerScore(i);

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

	return reward;
}

void MCTSPlayer::backup(MCTSPlayer::MCTSNode * v, RewardType const & delta)
{
	while (v != 0)
	{
		unapply(v, simGame);

		++N(v);

		// Not sure about this:
		for (int i = 0; i < delta.size(); ++i)
			v->rewards[i] += delta[i];

		v = v->parent;
	}
}

MCTSPlayer::MCTSTileNode * MCTSPlayer::generateTileNode(MCTSNode * parent, uchar player, int parentAction, Game const & g)
{
	Tile const * t = g.getTileByType(parentAction);
	MCTSTileNode * node = new MCTSTileNode(player, g.getPossibleTilePlacements(t), g.getPlayerCount(), parent, parentAction);
	return node;
}

MCTSPlayer::MCTSMeepleNode * MCTSPlayer::generateMeepleNode(MCTSNode * parent, uchar player, TileMove * parentAction, const Tile * t, const Game & g)
{
	MCTSMeepleNode * node;
	{
		MeepleMovesType possible;
		if (g.getPlayerMeeples(player) > 0)
			possible = g.getPossibleMeeplePlacements(t);
		else
			possible.push_back(MeepleMove());
		node = new MCTSMeepleNode(player, std::move(possible), g.getPlayerCount(), parent, parentAction);
	}
	return node;
}

MCTSPlayer::MCTSChanceNode *MCTSPlayer::generateChanceNode(MCTSNode * parent, uchar player, MeepleMove * parentAction, const Game & g)
{
	MCTSChanceNode * node = new MCTSChanceNode(player, g.getTileCounts(), g.getPlayerCount(), parent, parentAction);
	return node;
}
