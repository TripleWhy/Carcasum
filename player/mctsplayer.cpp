#include "mctsplayer.h"
#include "randomplayer.h"
#include <QElapsedTimer>

MCTSPlayer::MCTSNode::MCTSNode(uchar player, Type type, int size, MCTSNode * parent)
    : player(player),
      type(type),
      notExpanded(size),
      parent(parent)
{
	Q_ASSERT(size != 0);
	children.resize(size);
	children.shrink_to_fit();
}

MCTSPlayer::MCTSTileNode::MCTSTileNode(uchar player, TileMovesType && possible, MCTSNode * parent, int parentAction)
    : MCTSNode(player, TypeTile, possible.size(), parent),
      possible(possible),
      parentAction(parentAction)
{
}

MCTSPlayer::MCTSMeepleNode::MCTSMeepleNode(uchar player, MeepleMovesType && possible, MCTSNode * parent, TileMove * parentAction)
    : MCTSNode(player, TypeMeeple, possible.size(), parent),
      possible(possible),
      parentAction(parentAction)
{
}

MCTSPlayer::MCTSChanceNode::MCTSChanceNode(uchar player, TileCountType const & tileCounts, MCTSNode * parent, MeepleMove * parentAction)
    : MCTSNode(player, TypeChance, tileCounts.size(), parent),
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

template<typename NodeType>
void MCTSPlayer::apply(NodeType node, Game & g)
{
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
	syncGame();
}

TileMove MCTSPlayer::getTileMove(int player, const Tile * tile, const MoveHistoryEntry & /*move*/, const TileMovesType & /*placements*/)
{
	syncGame();

	//TODO use provided placements

	Q_ASSERT(game->equals(simGame));
	Q_ASSERT(simGame.getNextPlayer() == player);
	Q_UNUSED(player);
	MCTSTileNode * v0 = generateTileNode(0, tile->tileType, simGame);

	QElapsedTimer t;
	t.start();
#ifdef TIMEOUT
	do
#else
	for (int i = 0; i < M; ++i)
#endif
	{
//		qDebug() << i;
		Q_ASSERT(game->equals(simGame));

		auto vl = treePolicy(v0);
		auto delta = defaultPolicy(vl);
		backup(vl, delta);
		Q_ASSERT(game->equals(simGame));
	}
#ifdef TIMEOUT
	while (!t.hasExpired(TIMEOUT));
#endif
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
			auto r = expand(v);
			return r;
		}
		else
		{
			v = bestChild(v);
			apply(v, simGame);
		}
	}
	Q_ASSERT(v != 0);
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
			qreal val = (Q(vPrime) / qreal(N(vPrime))) + Cp * Util::mysqrt( Util::ln( N(v) ) / N(vPrime) );
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

RewardType MCTSPlayer::defaultPolicy(MCTSNode * v)
{
	switch (v->type)
	{
		case MCTSNode::TypeTile:
		{
			Tile const * tile = simGame.getTiles()[simGame.simEntry.tile];
			TileMove tileMove;
			TileMovesType && possibleTiles = simGame.getPossibleTilePlacements(tile);
			if (possibleTiles.size())
				tileMove = RandomPlayer::instance.getTileMove(v->player, tile, simGame.simEntry, possibleTiles);
			simGame.simPartStepTile(tileMove);

			MeepleMovesType && possibleMeeples = getPossibleMeeples(v->player, &tileMove, tile, simGame);
			simGame.simPartStepMeeple(RandomPlayer::instance.getMeepleMove(v->player, tile, simGame.simEntry, possibleMeeples));

			break;
		}
		case MCTSNode::TypeMeeple:
		{
			simGame.simPartStepMeeple(RandomPlayer::instance.getMeepleMove(v->player, 0, simGame.simEntry, static_cast<MCTSMeepleNode *>(v)->possible));
			break;
		}
		case MCTSNode::TypeChance:
			break;
	}

	int steps = 0;
	if (!simGame.isFinished())
	{
		do
			++steps;
		while (simGame.simStep(&RandomPlayer::instance));
	}
#if COUNT_PLAYOUTS
	++playouts;
#endif

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

	return reward;
}

void MCTSPlayer::backup(MCTSPlayer::MCTSNode * v, RewardType const & delta)
{
	while (true)
	{
		++N(v);
		Q(v) += delta[v->player];

		if (v->parent == 0)
			break;
		unapply(v, simGame);
		v = v->parent;
	}
}

void MCTSPlayer::syncGame()
{
	Util::syncGamesFast(*game, simGame);
}

MCTSPlayer::MCTSTileNode * MCTSPlayer::generateTileNode(MCTSNode * parent, int parentAction, Game & g)
{
	int player = g.getNextPlayer();
	Tile const * t = g.getTileByType(parentAction);
	apply(parentAction, g);
	TileMovesType && possible = g.getPossibleTilePlacements(t);
	if (possible.size() == 0)
		possible.push_back(TileMove()); // I could probably add a null MeepleMove child here, too.
	MCTSTileNode * node = new MCTSTileNode((uchar)player, std::move(possible), parent, parentAction);
	return node;
}

MCTSPlayer::MCTSMeepleNode * MCTSPlayer::generateMeepleNode(MCTSNode * parent, TileMove * parentAction, const Tile * t, Game & g)
{
	int player = g.getNextPlayer();
	apply(parentAction, g);
	MCTSMeepleNode * node;
	{
		MeepleMovesType && possible = getPossibleMeeples(player, parentAction, t, g);
		node = new MCTSMeepleNode((uchar)player, std::move(possible), parent, parentAction);
	}
	return node;
}

MCTSPlayer::MCTSChanceNode *MCTSPlayer::generateChanceNode(MCTSNode * parent, MeepleMove * parentAction, Game & g)
{
	int player = g.getNextPlayer();
	apply(parentAction, g);
	TileCountType const & tileCounts = g.getTileCounts();
	MCTSChanceNode * node = new MCTSChanceNode((uchar)player, tileCounts, parent, parentAction);

	return node;
}

