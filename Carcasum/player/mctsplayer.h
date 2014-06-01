#ifndef MCTSPLAYER_H
#define MCTSPLAYER_H

#include "static.h"
#include "playouts.h"
#include "utilities.h"
#include "core/player.h"
#include "core/game.h"
#include "core/random.h"
#include "core/tile.h"

template<class UtilityProvider = Utilities::PortionUtility, class Playout = Playouts::RandomPlayout>
class MCTSPlayer : public Player
{
	typedef typename UtilityProvider::RewardType RewardType;
	typedef typename UtilityProvider::RewardListType RewardListType;

private:
	struct MCTSNode
	{
		enum Type { TypeTile, TypeMeeple, TypeChance };

		uchar player;	// Does not really have to be stored
		Type type;
		uint notExpanded;
		uint visitCount = 0;
		RewardType reward = 0;

		std::vector<MCTSNode *> children;
		MCTSNode * parent;

		//needed only for progressive bias
		RewardType heuristicValue;

		uint childNSum()
		{
			uint sum = 0;
			for (auto c : children)
				if (c != 0)
					sum += c->visitCount;
			return sum;
		}

	protected:
		MCTSNode(uchar player, Type type, int size, MCTSNode * parent);
	};

	struct MCTSTileNode;
	struct MCTSMeepleNode;
	struct MCTSChanceNode;

	struct MCTSTileNode : public MCTSNode
	{
		TileMovesType possible;
		int parentAction;

		//needed only for progressive widening...
		std::vector<SimplePlayer3::RatingsNMeepleType> meepleRatings;

		MCTSTileNode(uchar player, TileMovesType && possible, MCTSNode * parent, int parentAction);
		~MCTSTileNode()
		{
			for (MCTSMeepleNode * c : *castChildren())
				delete c;
		}
		void deleteExcept(MCTSNode * n)
		{
			for (auto *& c : *castChildren())
			{
				if (c != 0 && c != n)
					c->deleteExcept(n);
				c = 0;
			}
			delete this;
		}
		int deleteCounting(int d)
		{
			int r = d+1;
			for (auto *& c : *castChildren())
			{
				if (c != 0)
					r = qMax(r, c->deleteCounting(d+1));
				c = 0;
			}
			delete this;
			return r;
		}

		inline std::vector<MCTSMeepleNode *> * castChildren() { return reinterpret_cast<std::vector<MCTSMeepleNode *> *>(&(MCTSNode::children)); }
		inline MCTSChanceNode * castParent() { return static_cast<MCTSChanceNode *>(MCTSNode::parent); }
	};

	struct MCTSMeepleNode : public MCTSNode
	{
		MeepleMovesType possible;
		TileMove * parentAction;

		MCTSMeepleNode(uchar player, MeepleMovesType && possible, MCTSNode * parent, TileMove * parentAction);
		~MCTSMeepleNode()
		{
			for (MCTSChanceNode * c : *castChildren())
				delete c;
		}
		void deleteExcept(MCTSNode * n)
		{
			for (auto *& c : *castChildren())
			{
				if (c != 0 && c != n)
					c->deleteExcept(n);
				c = 0;
			}
			delete this;
		}
		int deleteCounting(int d)
		{
			int r = d+1;
			for (auto *& c : *castChildren())
			{
				if (c != 0)
					r = qMax(r, c->deleteCounting(d+1));
				c = 0;
			}
			delete this;
			return r;
		}

		inline std::vector<MCTSChanceNode *> * castChildren() { return reinterpret_cast<std::vector<MCTSChanceNode *> *>(&(MCTSNode::children)); }
		inline MCTSTileNode * castParent() { return static_cast<MCTSTileNode *>(MCTSNode::parent); }
	};

	struct MCTSChanceNode : public MCTSNode
	{
		TileCountType tileCounts;
//		QVarLengthArray<MCTSTileNode *, TILE_COUNT_ARRAY_LENGTH> children;
		MeepleMove * parentAction;

		MCTSChanceNode(uchar player, const TileCountType & tileCounts, MCTSNode * parent, MeepleMove * parentAction);
		~MCTSChanceNode()
		{
			for (MCTSTileNode * c : *castChildren())
				delete c;
		}
		void deleteExcept(MCTSNode * n)
		{
			for (auto *& c : *castChildren())
			{
				if (c != 0 && c != n)
					c->deleteExcept(n);
				c = 0;
			}
			delete this;
		}
		int deleteCounting(int d)
		{
			int r = d+1;
			for (auto *& c : *castChildren())
			{
				if (c != 0)
					r = qMax(r, c->deleteCounting(d+1));
				c = 0;
			}
			delete this;
			return r;
		}

		inline std::vector<MCTSTileNode *> * castChildren() { return reinterpret_cast<std::vector<MCTSTileNode *> *>(&(MCTSNode::children)); }
		inline MCTSMeepleNode * castParent() { return static_cast<MCTSMeepleNode *>(MCTSNode::parent); }
	};

private:
	Game const * game;
	Game simGame = Game(0);
	jcz::TileFactory * tileFactory;
	RandomTable r;
	MeepleMove meepleMove;
	MCTSChanceNode * rootNode = 0;
	static Util::Math const & math;

	Playout playoutPolicy;
	UtilityProvider utilityProvider;
	QString typeName;
	const int M;
	const bool useTimeout;
	const qreal Cp;
	const bool reuseTree;
	const bool nodePriors;
	static constexpr uint nodePriorsInitiatPlayouts = MCTS_NODE_PRIORS_PLAYOUTS;
	const bool progressiveWidening;
	const bool progressiveBias;

#if MCTS_COUNT_EXPAND_HITS
public:
	uint hit = 0;
	uint miss = 0;
#endif

public:
#ifdef TIMEOUT
	constexpr MCTSPlayer(jcz::TileFactory * tileFactory, bool reuseTree = false, int const m = TIMEOUT, bool const mIsTimeout = true, qreal const Cp = 0.5, bool nodePriors = false, bool progressiveWidening = false, bool progressiveBias = false);
#else
	constexpr MCTSPlayer(jcz::TileFactory * tileFactory, bool reuseTree = false, int const m = 5000, bool const mIsTimeout = true, qreal const Cp = 0.5, bool nodePriors = false, bool progressiveWidening = false, bool progressiveBias = false);
#endif

	void applyChance(int action, Game & g);
	void applyTile(TileMove * action, Game & g);
	void applyMeeple(MeepleMove * action, Game & g);
	void applyNode(MCTSNode * node, Game & g);
	void unapplyChance(Game & g);
	void unapplyTile(Game & g);
	void unapplyMeeple(Game & g);
	void unapplyNode(MCTSNode * node, Game & g);

	virtual void newGame(int player, Game const * game);
	virtual void playerMoved(int player, Tile const * tile, MoveHistoryEntry const & move);
	virtual void undoneMove(MoveHistoryEntry const & move);
	virtual TileMove getTileMove(int player, Tile const * tile, MoveHistoryEntry const & move, TileMovesType const & placements);
	virtual MeepleMove getMeepleMove(int player, Tile const * tile, MoveHistoryEntry const & move, MeepleMovesType const & possible);
	virtual void endGame();
	virtual QString getTypeName() const;
	virtual Player * clone() const;

	MCTSNode * treePolicy(MCTSNode * node);
	MCTSNode * expand(MCTSNode * v);
	MCTSTileNode * expandChance(MCTSNode * v, int a);
	MCTSNode * bestChild(MCTSNode * v);
	int bestChild0(MCTSNode * v);
	RewardListType defaultPolicy(MCTSNode * v);
	void backup(MCTSNode * v, const RewardListType & delta);

	void syncGame();
	void fullSyncGame();

	inline constexpr RewardType & Q(MCTSNode * v) { return v->reward; }
	inline constexpr RewardType Q(MCTSNode const * v) const { return v->reward; }
	inline constexpr uint & N(MCTSNode * v) { return v->visitCount; }
	inline constexpr uint N(MCTSNode const * v) const { return v->visitCount; }
	inline constexpr uint NParent(MCTSNode const * v) const
	{
		return nodePriors ?
		            v->visitCount + (nodePriorsInitiatPlayouts * (uint)v->children.size()) - nodePriorsInitiatPlayouts
		          : v->visitCount;
	}
	inline constexpr bool expansionCandidate(MCTSNode * v) const
	{
		return progressiveWidening ?
//		            (v->notExpanded > 0) && ((int)v->visitCount >= ((1 << (v->children.size() - v->notExpanded)) / 4))
		            (v->notExpanded > 0) && ((int)v->visitCount >= ((1 << (v->children.size() - v->notExpanded)) - 1))
		          : v->notExpanded;
	}
	inline constexpr qreal UCB(MCTSNode const * v, MCTSNode const * vPrime) const
	{
		return progressiveBias ?
		            (qreal(Q(vPrime)) / qreal(N(vPrime)))  +  Cp * math.sqrt( math.ln( NParent(v) ) / N(vPrime) )  +  (vPrime->heuristicValue / (N(vPrime) + 1))
		          : (qreal(Q(vPrime)) / qreal(N(vPrime)))  +  Cp * math.sqrt( math.ln( NParent(v) ) / N(vPrime) );
	}

private:
	MCTSTileNode * generateTileNode(MCTSNode * parent, int parentAction, Game & g);
	MCTSMeepleNode * generateMeepleNode(MCTSNode * parent, TileMove * parentAction, const Tile * t, Game & g, int parentA);
	MCTSChanceNode * generateChanceNode(MCTSNode * parent, MeepleMove * parentAction, Game & g);

	RewardListType utilities(int const * scores, int const playerCount, Game const * g);

	inline MeepleMovesType getPossibleMeeples(int player, TileMove * parentAction, Tile const * t, Game & g)
	{
		MeepleMovesType possible;
		if (g.getPlayerMeeples(player) == 0 || parentAction->isNull())
			possible.push_back(MeepleMove());
		else
			possible = g.getPossibleMeeplePlacements(t);
		return possible;
	}
};

#include "mctsplayer.tpp"

#endif // MCTSPLAYER_H
