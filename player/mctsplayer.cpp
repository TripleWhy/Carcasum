#include "mctsplayer.h"
#include "randomplayer.h"
#include <QElapsedTimer>

MCTSPlayer::MCTSPlayer(jcz::TileFactory * tileFactory)
    : tileFactory(tileFactory)
{
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
	while (t.elapsed() < 30*1000)
	{
		auto vl = treePolicy(v0);
		auto delta = defaultPolicy();
		backup(vl, delta);
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

template<>
void MCTSPlayer::apply<MCTSPlayer::MCTSTileNode>(MCTSPlayer::MCTSTileNode * node, Game & g)
{
	g.simChanceStep(g.getTileIndexByType(node->parentAction));
}

template<>
void MCTSPlayer::apply<MCTSPlayer::MCTSMeepleNode>(MCTSPlayer::MCTSMeepleNode * node, Game & g)
{
	g.simTileStep(*node->parentAction);
}

template<>
void MCTSPlayer::apply<MCTSPlayer::MCTSChanceNode>(MCTSPlayer::MCTSChanceNode * node, Game & g)
{
	g.simMeepleStep(*node->parentAction);
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

MCTSPlayer::MCTSNode * MCTSPlayer::treePolicy(MCTSNode * v)
{
	while (!simGame.isFinished())
	{
		if (v->notExpanded)
			return expand(v);
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
		v->children[a] = vPrime;
	}

#if MCTS_COUNT_EXPAND_HITS
		--miss;
		++hit;
#endif

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
		qreal val = Q(vPrime) / qreal(N(vPrime));
		if (val > max)
		{
			max = val;
			a = i;
		}
	}
	return a;
}

int const * MCTSPlayer::defaultPolicy()
{
	while (simGame.simStep(&RandomPlayer::instance))
		;
	return simGame.getScores();
}

void MCTSPlayer::backup(MCTSPlayer::MCTSNode * v, const int * delta)
{
	// delta changes with undo.
	QVarLengthArray<int, MAX_PLAYERS> d(game->getPlayerCount());
	for (int i = 0; i < d.size(); ++i)
		d[i] = delta[i];

	while (v != 0)
	{
		++N(v);

		// Not sure about this:
		for (int i = 0; i < d.size(); ++i)
			v->rewards[i] += d[i];

		v = v->parent;
		simGame.undo();
	}
}

MCTSPlayer::MCTSTileNode * MCTSPlayer::generateTileNode(MCTSNode * parent, uchar player, int parentAction, Game const & g)
{
	MCTSTileNode * node = new MCTSTileNode();
	Tile const * t = game->getTileByType(parentAction);
	node->possible = g.getPossibleTilePlacements(t);
	int const size = node->possible.size();
	node->castChildren().resize(size);
	node->notExpanded = size;
	node->player = player;
	node->parent = parent;
	node->parentAction = parentAction;

	for (int i = 0; i < size; ++i)
	{
		Q_ASSERT(node->children[i] == 0);
		node->children[i] = 0;
	}

	return node;
}

MCTSPlayer::MCTSMeepleNode * MCTSPlayer::generateMeepleNode(MCTSNode * parent, uchar player, TileMove * parentAction, const Tile * t, const Game & g)
{
	MCTSMeepleNode * node = new MCTSMeepleNode();
	if (game->getPlayerMeeples(player) > 0)
		node->possible = g.getPossibleMeeplePlacements(t);
	else
		node->possible.push_back(MeepleMove());
	int const size = node->possible.size();
	node->castChildren().resize(size);
	node->notExpanded = size;
	node->player = player;
	node->parent = parent;
	node->parentAction = parentAction;

	return node;
}

MCTSPlayer::MCTSChanceNode *MCTSPlayer::generateChanceNode(MCTSNode * parent, uchar player, MeepleMove * parentAction, const Game & g)
{
	MCTSChanceNode * node = new MCTSChanceNode();
	node->tileCounts = g.getTileCounts();
	int const size = node->tileCounts.size();
	node->castChildren().resize(size);
	node->notExpanded = size;
	node->player = player;
	node->parent = parent;
	node->parentAction = parentAction;

	return node;
}
