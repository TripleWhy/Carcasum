#ifndef MCTSPLAYER_H
#define MCTSPLAYER_H

#include "static.h"
#include "core/player.h"
#include "core/game.h"
#include "core/random.h"
#include "core/tile.h"

class MCTSPlayer : public Player
{
public:
	static int const Cp = 1;

private:
	typedef QVarLengthArray<int, MAX_PLAYERS> RewardType;
	struct MCTSNode
	{
		enum Type { TypeTile, TypeMeeple, TypeChance };

		uchar player;	// Does not really have to be stored
		Type type;
		uint notExpanded;
		uint visitCount = 0;
		RewardType rewards;

		std::vector<MCTSNode *> children;
		MCTSNode * parent;

	protected:
		MCTSNode(uchar player, Type type, int size, int playerCount, MCTSNode * parent);
	};

	struct MCTSTileNode;
	struct MCTSMeepleNode;
	struct MCTSChanceNode;

	struct MCTSTileNode : public MCTSNode
	{
		TileMovesType possible;
//		QVarLengthArray<MCTSMeepleNode *, TILE_ARRAY_LENGTH> children;
//		Tile const * tile; // Well, this is what I tried to avoid...
		int parentAction;

		MCTSTileNode(uchar player, TileMovesType && possible, int playerCount, MCTSNode * parent, int parentAction);
		~MCTSTileNode()
		{
			for (MCTSMeepleNode * c : *castChildren())
				delete c;
		}

		inline std::vector<MCTSMeepleNode *> * castChildren() { return reinterpret_cast<std::vector<MCTSMeepleNode *> *>(&children); }
		inline MCTSChanceNode * castParent() { return static_cast<MCTSChanceNode *>(parent); }
	};

	struct MCTSMeepleNode : public MCTSNode
	{
		MeepleMovesType possible;
//		QVarLengthArray<MCTSChanceNode *, NODE_ARRAY_LENGTH> children;
		TileMove * parentAction;

		MCTSMeepleNode(uchar player, MeepleMovesType && possible, int playerCount, MCTSNode * parent, TileMove * parentAction);
		~MCTSMeepleNode()
		{
			for (MCTSChanceNode * c : *castChildren())
				delete c;
		}

		inline std::vector<MCTSChanceNode *> * castChildren() { return reinterpret_cast<std::vector<MCTSChanceNode *> *>(&children); }
		inline MCTSTileNode * castParent() { return static_cast<MCTSTileNode *>(parent); }
	};

	struct MCTSChanceNode : public MCTSNode
	{
		TileCountType tileCounts;
//		QVarLengthArray<MCTSTileNode *, TILE_COUNT_ARRAY_LENGTH> children;
		MeepleMove * parentAction;

		MCTSChanceNode(uchar player, const TileCountType & tileCounts, int playerCount, MCTSNode * parent, MeepleMove * parentAction);
		~MCTSChanceNode()
		{
			for (MCTSTileNode * c : *castChildren())
				delete c;
		}

		inline std::vector<MCTSTileNode *> * castChildren() { return reinterpret_cast<std::vector<MCTSTileNode *> *>(&children); }
		inline MCTSMeepleNode * castParent() { return static_cast<MCTSMeepleNode *>(parent); }
	};

private:
	Game const * game;
	Game simGame = Game(0);
	jcz::TileFactory * tileFactory;
	RandomTable r;
	MeepleMove meepleMove;

	bool print = false;
#if MCTS_COUNT_EXPAND_HITS
public:
	uint hit = 0;
	uint miss = 0;
#endif

public:
	MCTSPlayer(jcz::TileFactory * tileFactory);

	template<typename T>
	void apply(T t, Game & g);
	template<typename NodeType>
	void unapply(NodeType * node, Game & g);

	virtual void newGame(int player, Game const * game);
	virtual void playerMoved(int player, Tile const * tile, MoveHistoryEntry const & move);
	virtual TileMove getTileMove(int player, Tile const * tile, MoveHistoryEntry const & move, TileMovesType const & placements);
	virtual MeepleMove getMeepleMove(int player, Tile const * tile, MoveHistoryEntry const & move, MeepleMovesType const & possible);
	virtual void endGame();
	virtual char const * getTypeName();

	MCTSNode * treePolicy(MCTSNode * node);
	MCTSNode * expand(MCTSNode * v);
	MCTSNode * bestChild(MCTSNode * v);
	int bestChild0(MCTSNode * v);
	RewardType defaultPolicy(MCTSNode * v);
	void backup(MCTSNode * v, const RewardType & delta);

	inline int & Q(MCTSNode * v) { return v->rewards[v->player]; }
	inline uint & N(MCTSNode * v) { return v->visitCount; }
	inline qreal mySqrt(qreal r) { return sqrt(r); }
	inline qreal myLn(quint64 r) { return log(r); }

	inline uchar nextPlayer(uchar player) { return (player + 1) % game->getPlayerCount(); }

private:
	MCTSTileNode * generateTileNode(MCTSNode * parent, uchar player, int parentAction, Game & g);
	MCTSMeepleNode * generateMeepleNode(MCTSNode * parent, uchar player, TileMove * parentAction, const Tile * t, Game & g);
	MCTSChanceNode * generateChanceNode(MCTSNode * parent, uchar player, MeepleMove * parentAction, Game & g);

	void assertRewards(MCTSNode * n);
};

#endif // MCTSPLAYER_H