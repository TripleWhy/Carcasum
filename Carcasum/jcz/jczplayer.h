#if 1
#ifndef JCZPLAYER_H
#define JCZPLAYER_H

#include "core/player.h"
#include "core/game.h"
#include "core/board.h"

// Adaption of JCloisterZone's AI. Many parts where directly copied and adapted.

class JCZPlayer : public Player
{
public:
	JCZPlayer();
	virtual ~JCZPlayer();

	virtual void newGame(int player, Game const * game);
	virtual void playerMoved(int player, Tile const * tile, MoveHistoryEntry const & move);
	virtual TileMove getTileMove(int player, Tile const * tile, MoveHistoryEntry const & move, TileMovesType const & placements);
	virtual MeepleMove getMeepleMove(int player, Tile const * tile, MoveHistoryEntry const & move, MeepleMovesType const & possible);
	virtual void endGame();
	virtual QString getTypeName();
	virtual Player * clone() const;


private:
	static constexpr int OPEN_COUNT_ROAD = 0;
	static constexpr int OPEN_COUNT_CITY = 1;
	static constexpr int OPEN_COUNT_FARM = 2;
	static constexpr int OPEN_COUNT_CLOITSTER = 3;

protected:
	double rank(Game const & game, int const myIndex) {
		double ranking = 0;
//		initVars();
		int const packSize = game.getTileCount();
		int const enemyPlayers = game.getPlayerCount() - 1;
		int const myTurnsLeft = ((packSize-1) / (enemyPlayers+1)) + 1;
		Q_ASSERT(myTurnsLeft > 0);

//		//trigger score
//		game.getPhase().next(ScorePhase.class);
//		game.getPhase().enter();

		int openCount[4] = {};

		ranking += meepleRating(game, myIndex, enemyPlayers, myTurnsLeft);
		ranking += pointRating(game, myIndex, enemyPlayers);
//		ranking += openObjectRating();

//		ranking += rankPossibleFeatureConnections();
//		ranking += rankConvexity();
//		ranking += rankFairy();

		return ranking;
	}

	double reducePoints(double const points, int const player, int const myIndex, int const enemyPlayers) {
		if (player == myIndex) return points;
		return -points/enemyPlayers;
	}

	// I'm not sure, what this function actually computes, maybe it's an estimate of the chance value.
	double chanceToPlaceTile(QPoint pos, Game const & game) {
		EdgeMask pattern = game.getBoard()->getEdgeMask(pos);
		int openEdges = 0;
		for (TerrainType t : pattern.t)
			if (t == None)
				++openEdges;
		if (openEdges < 2) {
			uint remains = game.getPossibleTileCount(pattern);
			if (remains == 0) return 0.0;
			if (remains < game.getPlayerCount()) {
//				if (remains == 0) return 0.0;	//Does not make sense in my opinion
				return 1.0 - pow(1.0 - 1.0 / (game.getPlayerCount()), remains);
			}
		}
		return 1.0;
	}

	double meepleRating(Game const & game, int const myIndex, int const enemyPlayers, int const myTurnsLeft) {
		double rating = 0;

		uint const playerCount = game.getPlayerCount();
		for (int p = 0; p < playerCount; ++p) {
//			double meeplePoints = 0;
//			int limit = 0;

//			for (Follower f : Iterables.filter(p.getFollowers(), MeeplePredicates.deployed())) {
//				if (f instanceof SmallFollower) {
//					meeplePoints += 0.15;
//				} else if (f instanceof BigFollower) {
//					meeplePoints += 0.25;
//				}
//				if (++limit == myTurnsLeft) break;
//			}

			//This version of the loop above only works as long as we only have what jcz calls "SmallFollowers".
			//-->
			int placed = game.getPlacedMeeples(p);
			double meeplePoints;
			if (placed >= myTurnsLeft)
				meeplePoints = 0.15 * qMin(placed, myTurnsLeft);
			//<--

			rating += reducePoints(meeplePoints, p, myIndex, enemyPlayers);
		}
		return rating;
	}

	double pointRating(Game const & game, int const myIndex, int const enemyPlayers) {
		double rating = 0;

		int const playerCount = game.getPlayerCount();
		for (int p = 0; p < playerCount; ++p) {
			rating += reducePoints(game.getPlayerScore(p), p, myIndex, enemyPlayers);
		}

//		ScoreAllFeatureFinder scoreAll = new ScoreAllFeatureFinder();
//		LegacyAiScoreAllCallback callback = new LegacyAiScoreAllCallback();
//		scoreAll.scoreAll(game, callback);
//		rating += callback.getRanking();

		return rating;
	}
};

#endif // JCZPLAYER_H
#endif
