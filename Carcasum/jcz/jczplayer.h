#ifndef JCZPLAYER_H
#define JCZPLAYER_H

#include "core/player.h"
#include "core/game.h"
#include "core/board.h"
#include <unordered_set>
#include <array>

#define JCZ_DEBUG_PRINTS 0

// Adaption of JCloisterZone's AI. Many parts where directly copied and adapted.
// Some stuff is rewritten and does not include other stuff used for extensions.

namespace jcz
{

class JCZPlayer : public Player
{
private:
	Game const * game;
	Game simGame = Game(0);
	jcz::TileFactory * tileFactory;
	MeepleMove meepleMove;

	int player = -1;
	uint playerCount = 0;
	int enemyPlayers = 0;
	QHash<QPoint, double> chanceCachePos;
	QHash<Node const *, double> chanceCacheNode;

public:
	JCZPlayer(jcz::TileFactory * tileFactory);
	virtual ~JCZPlayer();

	virtual void newGame(int player, Game const * game);
	virtual void playerMoved(int player, Tile const * tile, MoveHistoryEntry const & move);
	virtual TileMove getTileMove(int player, Tile const * tile, MoveHistoryEntry const & move, TileMovesType const & placements);
	virtual MeepleMove getMeepleMove(int player, Tile const * tile, MoveHistoryEntry const & move, MeepleMovesType const & possible);
	virtual void endGame();
	virtual QString getTypeName();
	virtual Player * clone() const;

#if JCZ_DEBUG_PRINTS
private:
	void printRank(char const * prefix, const TileMove * tm, const MeepleMove & mm, double rank);
#endif

private:
	static constexpr std::array<std::pair<Tile::Side, QPoint>, 4> ADJACENT = {{
	                                                                              {Tile::up,    QPoint( 0, -1)},
	                                                                              {Tile::right, QPoint(+1,  0)},
	                                                                              {Tile::down,  QPoint( 0, +1)},
	                                                                              {Tile::left,  QPoint(-1,  0)}
	                                                                         }};
	static constexpr std::array<QPoint, 8> ADJACENT_AND_DIAGONAL = {{
	                                                                    {QPoint(0, -1)},
	                                                                    {QPoint(+1, 0)},
	                                                                    {QPoint(0, +1)},
	                                                                    {QPoint(-1, 0)},
	                                                                    {QPoint(+1, -1)},
	                                                                    {QPoint(+1, +1)},
	                                                                    {QPoint(-1, +1)},
	                                                                    {QPoint(-1, -1)}
	                                                               }};

	static constexpr double TRAPPED_MY_FIGURE_POINTS = -12.0;
	static constexpr double TRAPPED_ENEMY_FIGURE_POINTS = 3.0;
	static constexpr double SELF_MERGE_PENALTY = 6.0;

	static constexpr double MIN_CHANCE = 0.4;

	static constexpr int OPEN_COUNT_ROAD = 0;
	static constexpr int OPEN_COUNT_CITY = 1;
	static constexpr int OPEN_COUNT_FARM = 2;
	static constexpr int OPEN_COUNT_CLOITSTER = 3;

	static constexpr std::array<std::array<double, 9>, 4> OPEN_PENALTY = {{
	    {{ 0.0, 1.0, 2.5, 4.5, 7.5, 10.5, 14.5, 19.0, 29.0 }}, //road
	    {{ 0.0, 0.5, 1.5, 3.0, 5.0, 8.0, 12.0, 17.0, 27.0 }}, //city
	    {{ 0.0, 5.0, 10.0, 19.0, 28.0, 37.0, 47.0, 57.0, 67.0 }}, //farm
	    {{ 0.0, 0.0, 0.4, 0.8, 1.2, 2.0, 4.0, 7.0, 11.0 }} //cloister
	}};

protected:
	struct RankData
	{
		Game const * game;
		int const packSize;
		int const myTurnsLeft;
		int openCount[4];
		Tile const * tile;
		const QPoint placement;
	};

	double rank(Game const * game, Tile const * tile, QPoint const & placement);
	double reducePoints(double const points, int const player);
	double chanceToPlaceTile(RankData const & data, QPoint pos);
	double meepleRating(RankData & data);
	double pointRating(RankData & data);
	double openObjectRating(RankData & data);
	double rankPossibleFeatureConnections(RankData & data);
	double rankConvexity(RankData const & data);

private:
	double scoreAllRanking(RankData & data);
	void scoreCompletableFeature(RankData & data, Node const * n, double & rnk);
	void scoreFarm(RankData & data, Node const * n, double & rnk);
	double getFarmPoints(RankData & data, FieldNode const * farm, int p);
	double rankUnfishedCompletable(RankData & data, Node const * n);
	double getUnfinishedCompletablePoints(RankData & data, Node const * n);
	double getCompletableChanceToClose(RankData & data, Node const * completable);
	double getCloisterChanceToClose(RankData const & data);
	double getUnfinishedCityPoints(RankData & data, CityNode const * city);
	double getUnfinishedRoadPoints(RankData & data, RoadNode const * road);
	double getUnfinishedCloisterPoints(RankData & data, CloisterNode const * cloister);
	double rankTrappedMeeples(RankData & data, Node const * n);

	double futureConnectionRateConnection(RankData & data, Tile::Side const & toEmpty, Tile::Side const & toFeature, Tile const * tile1, Tile const * tile2, double chance);
	double futureConnectionRateFeatures(RankData & data, Tile::Side const & toEmpty, Tile::Side const & toFeature, double chance, Node const * f1, Node const * f2);
	double futureConnectionGetFeaturePoints(RankData & data, Node const * feature);
	void funtureConnectionSumPower(uchar & myPower, uchar & bestEnemy, Node const * f1, Node const * f2);
	double futureConnectionRateCrossing(Node const * f1, Node const * f2);
};

}

#endif // JCZPLAYER_H
