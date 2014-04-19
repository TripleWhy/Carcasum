#include "mctsplayer.h"

template<>
template<>
void MCTSPlayer<false>::apply<int>(int action, Game & g)
{
	g.simPartStepChance(g.getTileIndexByType(action));
}

template<>
template<>
void MCTSPlayer<true>::apply<int>(int action, Game & g)
{
	g.simPartStepChance(g.getTileIndexByType(action));
}

template<>
template<>
void MCTSPlayer<false>::apply<TileMove *>(TileMove * action, Game & g)
{
	g.simPartStepTile(*action);
}

template<>
template<>
void MCTSPlayer<true>::apply<TileMove *>(TileMove * action, Game & g)
{
	g.simPartStepTile(*action);
}

template<>
template<>
void MCTSPlayer<false>::apply<MeepleMove *>(MeepleMove * action, Game & g)
{
	g.simPartStepMeeple(*action);
}

template<>
template<>
void MCTSPlayer<true>::apply<MeepleMove *>(MeepleMove * action, Game & g)
{
	g.simPartStepMeeple(*action);
}

MCTS_T
template<typename NodeType>
void MCTSPlayer MCTS_TU::apply(NodeType node, Game & g)
{
	apply(node->parentAction, g);
}

template<>
template<>
void MCTSPlayer<false>::apply<MCTSPlayer<false>::MCTSNode *>(MCTSNode * node, Game & g)
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
template<>
void MCTSPlayer<true>::apply<MCTSPlayer<true>::MCTSNode *>(MCTSNode * node, Game & g)
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
template<>
void MCTSPlayer<false>::unapply<MCTSPlayer<false>::MCTSTileNode>(MCTSPlayer::MCTSTileNode * /*node*/, Game & g)
{
	g.simPartUndoChance();
}

template<>
template<>
void MCTSPlayer<true>::unapply<MCTSPlayer<true>::MCTSTileNode>(MCTSPlayer::MCTSTileNode * /*node*/, Game & g)
{
	g.simPartUndoChance();
}

template<>
template<>
void MCTSPlayer<false>::unapply<MCTSPlayer<false>::MCTSMeepleNode>(MCTSPlayer::MCTSMeepleNode * /*node*/, Game & g)
{
	g.simPartUndoTile();
}

template<>
template<>
void MCTSPlayer<true>::unapply<MCTSPlayer<true>::MCTSMeepleNode>(MCTSPlayer::MCTSMeepleNode * /*node*/, Game & g)
{
	g.simPartUndoTile();
}

template<>
template<>
void MCTSPlayer<false>::unapply<MCTSPlayer<false>::MCTSChanceNode>(MCTSPlayer::MCTSChanceNode * /*node*/, Game & g)
{
	g.simPartUndoMeeple();
}

template<>
template<>
void MCTSPlayer<true>::unapply<MCTSPlayer<true>::MCTSChanceNode>(MCTSPlayer::MCTSChanceNode * /*node*/, Game & g)
{
	g.simPartUndoMeeple();
}

template<>
template<>
void MCTSPlayer<false>::unapply<MCTSPlayer<false>::MCTSNode>(MCTSPlayer::MCTSNode * node, Game & g)
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

template<>
template<>
void MCTSPlayer<true>::unapply<MCTSPlayer<true>::MCTSNode>(MCTSPlayer::MCTSNode * node, Game & g)
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

template<>
MCTSPlayer<false>::RewardListType MCTSPlayer<false>::utilities(const int * scores, const int playerCount)
{
	return Util::utilitySimpleMulti(scores, playerCount);
}

template<>
MCTSPlayer<true>::RewardListType MCTSPlayer<true>::utilities(const int * scores, const int playerCount)
{
	auto const & u = Util::utilityComplexMulti(scores, playerCount);
	RewardListType r(u.size());
	for (int i = 0; i < u.size(); ++i)
		r[i] = utilityMap[u[i]];
	return r;
}

inline static void newGameShare(int /*player*/, Game const * g, Game & simGame, jcz::TileFactory * tileFactory)
{
	simGame.clearPlayers();
	for (uint i = 0; i < g->getPlayerCount(); ++i)
		simGame.addPlayer(&RandomPlayer::instance);
	simGame.newGame(g->getTileSets(), tileFactory, g->getMoveHistory());
}

template<>
void MCTSPlayer<false>::newGame(int player, const Game * g)
{
	game = g;
	newGameShare(player, g, simGame, tileFactory);
}

template<>
void MCTSPlayer<true>::newGame(int player, const Game * g)
{
	game = g;
	newGameShare(player, g, simGame, tileFactory);

	utilityMap = Util::getUtilityMap(g);
}

template<>
const char * MCTSPlayer<true>::getTypeName()
{
	return "MCTSPlayer<true>";
}

template<>
const char * MCTSPlayer<false>::getTypeName()
{
	return "MCTSPlayer<false>";
}
