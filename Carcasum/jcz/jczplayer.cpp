#include "jczplayer.h"
#include "player/randomplayer.h"

constexpr std::array<std::pair<Tile::Side, QPoint>, 4> jcz::JCZPlayer::ADJACENT;
constexpr std::array<QPoint, 8> jcz::JCZPlayer::ADJACENT_AND_DIAGONAL;
constexpr std::array<std::array<double, 9>, 4> jcz::JCZPlayer::OPEN_PENALTY;

jcz::JCZPlayer::JCZPlayer(jcz::TileFactory * tileFactory)
    : tileFactory(tileFactory)
{
}

jcz::JCZPlayer::~JCZPlayer()
{

}

void jcz::JCZPlayer::newGame(int p, const Game * g)
{
	player = p;
	playerCount = g->getPlayerCount();
	enemyPlayers = g->getPlayerCount() - 1;

	endGame();
	game = g;
	simGame.clearPlayers();
	for (uint i = 0; i < g->getPlayerCount(); ++i)
		simGame.addPlayer(&RandomPlayer::instance);
	simGame.newGame(g->getTileSets(), tileFactory, g->getMoveHistory());
}

void jcz::JCZPlayer::playerMoved(int /*player*/, const Tile * /*tile*/, const MoveHistoryEntry & /*move*/)
{
	Util::syncGamesFast(*game, simGame);
}

void jcz::JCZPlayer::undoneMove(const MoveHistoryEntry & /*move*/)
{
	Util::syncGames(*game, simGame);
}

#if JCZ_DEBUG_PRINTS
#include <iostream>
#include <iomanip>
#include <boost/lexical_cast.hpp>
void jcz::JCZPlayer::printRank(char const * prefix, TileMove const * tm, MeepleMove const & mm, double rank)
{
	std::locale::global(std::locale::classic());
	int offset = game->getBoard()->getOffset();
	char const * rotations[4] = {"R0", "R90", "R180", "R270"};

	std::cout << prefix << "[NioProcessor-12] INFO com.jcloisterzone.ai.legacyplayer.LegacyAiPlayer - Rank BA.xxx > "
	          << boost::lexical_cast<std::string>(rank) << " Pos: [x=" << ((int)tm->x - offset) << ",y=" << ((int)tm->y - offset) << "] Rot: " << rotations[tm->orientation]
	          << " Meeple: SmallFollower APos: [x=" << ((int)tm->x - offset) << ",y=" << ((int)tm->y - offset) << "] ALoc: " << (int)mm.nodeIndex << std::endl;
}
#endif

TileMove jcz::JCZPlayer::getTileMove(int p, const Tile * tile, const MoveHistoryEntry & move, const TileMovesType & placements)
{
	if (p != player)
		return RandomPlayer::instance.getTileMove(p, tile, move, placements);

	Util::syncGamesFast(*game, simGame);

	Q_ASSERT(game->equals(simGame));
	Q_ASSERT(simGame.getNextPlayer() == p);

	TileMove const * bestSoFar = 0;
	MeepleMove bestSoFarMeeple;
	double bestSoFarRank = -std::numeric_limits<double>::infinity();

	simGame.simPartStepChance(move.tileIndex);
	for (TileMove const & tileMove : placements)
	{
		QPoint const placement(tileMove.x, tileMove.y);
		simGame.simPartStepTile(tileMove);
		Tile const * simTile = simGame.simTile;

		//JCZ checks not placing a meeple last, while my game puts it first. This results in JCZ implicitly preferring meeple placements over not placing meeples. Thus I need to shift the order a bit.
		auto const & meeplePlacements = game->getPossibleMeeplePlacements(p, simTile);
		for (int i = 0, s = meeplePlacements.size(); i < s; ++i)
		{
			MeepleMove const & meepleMove = meeplePlacements[(i + 1) % s];
			simGame.simPartStepMeeple(meepleMove);

			double rnk = rank(&simGame, simTile, placement);
			if (rnk > bestSoFarRank)	//TODO choose randomly between equal rankings? JCZ does not do this. But I could.
			{
				bestSoFarRank = rnk;
				bestSoFar = &tileMove;
				bestSoFarMeeple = meepleMove;
			}

//			printRank("\t", &tileMove, meepleMove, rnk);

			simGame.simPartUndoMeeple();
		}
		simGame.simPartUndoTile();
	}
	simGame.simPartUndoChance();

#if JCZ_DEBUG_PRINTS
	printRank("", bestSoFar, bestSoFarMeeple, bestSoFarRank);
#endif

	Q_ASSERT(game->equals(simGame));
	Q_ASSERT(bestSoFar != 0);

	meepleMove = bestSoFarMeeple;
	if (bestSoFar == 0)
	{
		qWarning("jcz::JCZPlayer::getTileMove:  bestSoFar == 0");
		return RandomPlayer::instance.getTileMove(p, tile, move, placements);
	}
	else
		return *bestSoFar;
}

MeepleMove jcz::JCZPlayer::getMeepleMove(int p, const Tile * /*tile*/, const MoveHistoryEntry & /*move*/, const MeepleMovesType & /*possible*/)
{
	if (p != player)
		return MeepleMove();
	return meepleMove;
}

void jcz::JCZPlayer::endGame()
{
}

QString jcz::JCZPlayer::getTypeName() const
{
	return "JCZPlayer";
}

Player * jcz::JCZPlayer::clone() const
{
	return new JCZPlayer(tileFactory);
}

double jcz::JCZPlayer::rank(const Game * game, const Tile * tile, const QPoint & placement) {
	chanceCachePos.clear();
	chanceCacheNode.clear();

	double ranking = 0.0;

	int const packSize = game->getTileCount();
	int const myTurnsLeft = ((packSize-1) / (enemyPlayers+1)) + 1;
	Q_ASSERT(myTurnsLeft > 0);

	RankData data { game, packSize, myTurnsLeft, {0, 0, 0, 0}, tile, placement };

#if !JCZ_DEBUG_PRINTS
	ranking += meepleRating(data);
	ranking += pointRating(data);
	ranking += openObjectRating(data);

	ranking += rankPossibleFeatureConnections(data);
	ranking += rankConvexity(data);
//	ranking += rankFairy();
#else
	int offset = game->getBoard()->getOffset();

	double mr = meepleRating(data);
	double pr = pointRating(data);
	double oor = openObjectRating(data);

	double rpfc = rankPossibleFeatureConnections(data);
	double rc = rankConvexity(data);
	ranking = mr + pr + oor + rpfc + rc;

	std::locale::global(std::locale::classic());
	static char const * rotations[4] = {"R0", "R90", "R180", "R270"};
	std::stringstream pos;
	pos << "[x=" << (placement.x() - offset) << ",y=" << (placement.y() - offset) << "]";
	std::cout << "\t\t" << std::setw(13) << pos.str() << std::setw(4) << rotations[tile->orientation] << " -> "
	          << boost::lexical_cast<std::string>(ranking) << "  =  "
	          << boost::lexical_cast<std::string>(mr) << " " << boost::lexical_cast<std::string>(pr) << " " << boost::lexical_cast<std::string>(oor) << " "
	          << boost::lexical_cast<std::string>(rpfc) << " " << boost::lexical_cast<std::string>(rc) << " " << boost::lexical_cast<std::string>(0.0) << std::endl;
#endif

	return ranking;
}

double jcz::JCZPlayer::reducePoints(const double points, const int p) {
	if (p == player) return points;
	return -points/enemyPlayers;
}

// I'm not sure, what this function actually computes, maybe it's an estimate of the chance value.
double jcz::JCZPlayer::chanceToPlaceTile(jcz::JCZPlayer::RankData const & data, QPoint pos) {
	if (chanceCachePos.contains(pos))
		return chanceCachePos.value(pos);

	if (data.game->getBoard()->getTile(pos) != 0)
		return 1.0;

	EdgeMask pattern = data.game->getBoard()->getEdgeMask(pos);
	int openEdges = 0;
	for (TerrainType t : pattern.t)
		if (t == None)
			++openEdges;
	if (openEdges < 2) {
		uint remains = data.game->getPossibleTileCount(pattern);
		if (remains == 0) return 0.0;
		if (remains < playerCount) {
//			if (remains == 0) return 0.0;	//Does not make sense in my opinion
			return 1.0 - pow(1.0 - 1.0 / (playerCount), remains);
		}
	}
	return 1.0;
}

double jcz::JCZPlayer::meepleRating(jcz::JCZPlayer::RankData & data) {
	double rating = 0;

	for (uint p = 0; p < playerCount; ++p) {
//		double meeplePoints = 0;
//		int limit = 0;

//		for (Follower f : Iterables.filter(p.getFollowers(), MeeplePredicates.deployed())) {
//			if (f instanceof SmallFollower) {
//				meeplePoints += 0.15;
//			} else if (f instanceof BigFollower) {
//				meeplePoints += 0.25;
//			}
//			if (++limit == myTurnsLeft) break;
//		}

		//This version of the loop above only works as long as we only have what jcz calls "SmallFollowers".
		//-->
		int placed = data.game->getPlacedMeeples(p);
		double meeplePoints = 0.15 * qMin(placed, data.myTurnsLeft);
		//<--

		rating += reducePoints(meeplePoints, p);
	}
	return rating;
}

double jcz::JCZPlayer::scoreAllRanking(jcz::JCZPlayer::RankData & data)
{
	double rank = 0;
	Board const * board = data.game->getBoard();
	std::vector<Tile const *> const & tiles = board->getTiles();
	std::unordered_set<Node::NodeData const *> scored;

	for (Tile const * tile : tiles)
	{
		for (Node const * const * np = tile->getCNodes(), * const * end = np + tile->getNodeCount(); np < end; ++np)
		{
			Node const * n = *np;
			if (!n->isOccupied())
				continue;
			if (n->getScored() != NotScored)
				continue;
			if (scored.find(n->getData()) != scored.end())
				continue;
			scored.insert(n->getData());

			if (n->getTerrain() == Field)
				scoreFarm(data, n, rank);
			else
				scoreCompletableFeature(data, n, rank);
		}
	}

	return rank;
}

void jcz::JCZPlayer::scoreCompletableFeature(jcz::JCZPlayer::RankData & data, const Node * n, double & rnk)
{
#if !JCZ_DEBUG_PRINTS
	rnk += rankUnfishedCompletable(data, n);
	rnk += rankTrappedMeeples(data, n);
//	rnk += legacyAiRankSpecialFigures(n);
#else
	//No printing, but still useful for debugging.
	double r1 = rankUnfishedCompletable(data, n);
	double r2 = rankTrappedMeeples(data, n);
	rnk += r1 + r2;
#endif
}

void jcz::JCZPlayer::scoreFarm(jcz::JCZPlayer::RankData & data, const Node * n, double & rnk)
{
//	if (isInTowerDanger(ctx)) return;
	auto farm = static_cast<FieldNode const *>(n);
	int max = farm->getMaxMeeples();
	for (uint p = 0; p < playerCount; ++p)
	{
		if (farm->getPlayerMeeples(p) != max)
			continue;
		double points = getFarmPoints(data, farm, p);
		rnk += reducePoints(points, p);
	}
}

double jcz::JCZPlayer::getFarmPoints(jcz::JCZPlayer::RankData & data, const FieldNode * farm, int p) {
	if (p == player) {
		data.openCount[OPEN_COUNT_FARM]++;
	}
	return farm->getScore();
}

double jcz::JCZPlayer::rankUnfishedCompletable(jcz::JCZPlayer::RankData & data, const Node * n) {
	double rating = 0.0;
	double points = getUnfinishedCompletablePoints(data, n);
	int max = n->getMaxMeeples();
	for (uint p = 0; p < playerCount; ++p)
	{
		if (n->getPlayerMeeples(p) != max)
			continue;
		rating += reducePoints(points, p);
	}
	return rating;
}

double jcz::JCZPlayer::getUnfinishedCompletablePoints(jcz::JCZPlayer::RankData & data, const Node * n) {
	switch (n->getTerrain())
	{
		case City:
			return getUnfinishedCityPoints(data, static_cast<CityNode const *>(n));
		case Road:
			return getUnfinishedRoadPoints(data, static_cast<RoadNode const *>(n));
		case Cloister:
			return getUnfinishedCloisterPoints(data, static_cast<CloisterNode const *>(n));
		default:
			return 0;
	}
}

double jcz::JCZPlayer::getCompletableChanceToClose(jcz::JCZPlayer::RankData & data, const Node * completable) {
	Q_ASSERT(completable->getTerrain() != Field);
	if (chanceCacheNode.contains(completable))
		return chanceCacheNode.value(completable);

	double result = 1.0;
	if (completable->getTerrain() == Cloister)
		result = getCloisterChanceToClose(data);
	else
	{
		QSet<QPoint> openEdgesChanceToClose;
		Board const * board = data.game->getBoard();
		Tile const * lastTile = 0;
		for (Tile const * tile : completable->getData()->tiles)
		{
			if (tile == lastTile)
				continue;
			lastTile = tile;

			//TODO this is incredibly slow! Finds open positions for the node.
			QPoint pos = board->positionOf(tile);
			for (auto const & d : ADJACENT)
			{
				QPoint p = pos + d.second;
				if (board->getTile(p) == 0)
				{
					//side is open
					if (!openEdgesChanceToClose.contains(p))
					{
						double chanceToClose = chanceToPlaceTile(data, p);
						openEdgesChanceToClose.insert(p);
						result *= chanceToClose;
					}
				}
			}
		}
		if (result > 0.95)
			result = 0.95;
	}
	chanceCacheNode.insert(completable, result);
	return result;
}

double jcz::JCZPlayer::getCloisterChanceToClose(jcz::JCZPlayer::RankData const & data) {
	double result = 1.0;
	for (QPoint const & d : ADJACENT_AND_DIAGONAL)
	{
		QPoint const & adjacent = data.placement + d;
		result *= chanceToPlaceTile(data, adjacent);
	}
	//for "1.6-compatibility" - make it already sense ?
	if (result > 0.85) return 0.85;
	return result;
}

double jcz::JCZPlayer::getUnfinishedCityPoints(jcz::JCZPlayer::RankData & data, const CityNode * city) {
	double chanceToClose = getCompletableChanceToClose(data, city);

	if (chanceToClose > MIN_CHANCE && city->getPlayerMeeples(player) == city->getMaxMeeples()) {
		data.openCount[OPEN_COUNT_CITY]++;
	}

	//legacy heuristic
	if (chanceToClose < MIN_CHANCE) {
		return city->getScore() + 3.0*chanceToClose;
	} else {
		int points = city->getScore();
		if (city->uniqueTileCount() > 2)	//TODO this should not be done here...
			points *= 2;
		return points - 3.0*(1.0-chanceToClose);
	}
}

double jcz::JCZPlayer::getUnfinishedRoadPoints(jcz::JCZPlayer::RankData & data, const RoadNode * road) {
	double chanceToClose = getCompletableChanceToClose(data, road);

	if (chanceToClose > MIN_CHANCE && road->getPlayerMeeples(player) == road->getMaxMeeples()) {
		data.openCount[OPEN_COUNT_ROAD]++;
	}

	//legacy heuristic
	if (chanceToClose < MIN_CHANCE) {
		return road->getScore() + 3.0*chanceToClose;
	} else {
		return road->getScore() - 3.0*(1.0-chanceToClose);
	}

}

double jcz::JCZPlayer::getUnfinishedCloisterPoints(jcz::JCZPlayer::RankData & data, const CloisterNode * cloister) {
	if (cloister->getPlayerMeeples(player) != 0) {
		data.openCount[OPEN_COUNT_CLOITSTER]++;
	}
	double chanceToClose = getCompletableChanceToClose(data, cloister);
	int points = cloister->getScore();
	return points + (9-points)*chanceToClose;
}

double jcz::JCZPlayer::rankTrappedMeeples(jcz::JCZPlayer::RankData & data, const Node * n) {
	//musi tu byt dolni mez - btw nestaci toto misto hodnoceni figurek, spis asi :)

	//jczTODO lepe
	if (data.myTurnsLeft < 8) return 0.0;

	double chanceToClose = getCompletableChanceToClose(data, n);
	if (chanceToClose > 0.4) return 0.0;

	double rating = 0.0;
	for (int i = 0; i < (int)playerCount; ++i)
	{
		if (i == player) {
			rating += n->getPlayerMeeples(i) * TRAPPED_MY_FIGURE_POINTS;
		} else {
			rating += n->getPlayerMeeples(i) * TRAPPED_ENEMY_FIGURE_POINTS;
		}
	}
	return (1.0 - chanceToClose) * rating; //no reduce
}

double jcz::JCZPlayer::pointRating(jcz::JCZPlayer::RankData & data) {
	double rating = 0;

	for (uint p = 0; p < playerCount; ++p) {
		rating += reducePoints(data.game->getPlayerScore(p), p);
	}

//	ScoreAllFeatureFinder scoreAll = new ScoreAllFeatureFinder();
//	LegacyAiScoreAllCallback callback = new LegacyAiScoreAllCallback();
//	scoreAll.scoreAll(game, callback);
//	rating += callback.getRanking();
	rating += scoreAllRanking(data);

	return rating;
}

double jcz::JCZPlayer::openObjectRating(jcz::JCZPlayer::RankData & data) {
	double rating = 0;

	for (uint i = 0; i < OPEN_PENALTY.size(); i++ ){
		double penalty;
		//fast fix for strange bug causes ArrayIndexOutOfBoundsException: 9
		if (data.openCount[i] >= (int)OPEN_PENALTY[i].size()) {
			penalty = OPEN_PENALTY[i][OPEN_PENALTY[i].size() - 1];
		} else {
			penalty = OPEN_PENALTY[i][data.openCount[i]];
		}
		if (i == 2) {
			//Farm
			double modifier = (data.packSize - ((1+enemyPlayers) * 3)) / 20.0;
			if (modifier < 1.0) modifier = 1.0;
			rating -= modifier * penalty;
		} else {
			rating -= penalty;
		}
	}
	return rating;
}

double jcz::JCZPlayer::rankPossibleFeatureConnections(jcz::JCZPlayer::RankData & data) {
	double rank = 0;

	Board const * board = data.game->getBoard();
	for (auto const & eplace : ADJACENT)
	{
		QPoint const pos = data.placement + eplace.second;
		if (board->getTile(pos) != 0) continue;

		double chance = chanceToPlaceTile(data, pos);
		if (chance < MIN_CHANCE) continue;

		for (auto const & econn : ADJACENT)
		{
			QPoint const conn = pos + econn.second;
			if (conn == data.placement) continue;
			Tile const * connTile = board->getTile(conn);
			if (connTile == 0) continue;

			rank += futureConnectionRateConnection(data, eplace.first, econn.first, data.tile, connTile, chance);
		}
	}
	return rank;
}

double jcz::JCZPlayer::futureConnectionRateConnection(jcz::JCZPlayer::RankData & data, const Tile::Side & toEmpty, const Tile::Side & toFeature, const Tile * tile1, const Tile * tile2, double chance) {
	double rating = 0;

	Tile::Side const & oppFeat = Tile::sideOpposite(toFeature);
	Node const * f1 = tile1->getFeatureNode(toEmpty);
	Node const * f2 = tile2->getFeatureNode(oppFeat);

	if (f1->getTerrain() != Field && f2->getTerrain() != Field) {
		if (f1->getTerrain() == f2->getTerrain()) {
//            System.err.println("    " + tile1.getPosition() + " <-->" + f2Pos + " / " + f1 + " " + f2);
			rating +=  futureConnectionRateFeatures(data, toEmpty, toFeature, chance, f1, f2);
		} else {
			rating +=  futureConnectionRateCrossing(f1, f2);
		}
	}

	if (toEmpty != toFeature) {
		bool left = (Tile::sideRotateCCW(toEmpty) == toFeature);
		FieldNode const * farm1 = static_cast<FieldNode const *>( tile2->getFieldNode(toEmpty, left ? 2 : 0) );
		FieldNode const * farm2 = static_cast<FieldNode const *>( tile2->getFieldNode(oppFeat, left ? 0 : 2) );

		if (farm1 != 0 && farm2 != 0) {
//                System.err.println("    " + tile1.getPosition() + " <-->" + f2Pos + " / " + farm1 + " " + farm2);
			rating +=  futureConnectionRateFeatures(data, toEmpty, toFeature, chance, farm1, farm2);
		}
	}
	return rating;
}

double jcz::JCZPlayer::futureConnectionRateFeatures(jcz::JCZPlayer::RankData & data, const Tile::Side & toEmpty, const Tile::Side & toFeature, double chance, const Node * f1, const Node * f2) {
	if (!f1->isOccupied()) {
		return 0;
	}

	uchar myPower = 0, bestEnemy;
	funtureConnectionSumPower(myPower, bestEnemy, f1, f2);

	if (bestEnemy == 0  &&  f1->getPlayerMeeples(player) == 1  &&  f2->getPlayerMeeples(player) == 1)
	{
		return -SELF_MERGE_PENALTY;
	}

	double myPoints = futureConnectionGetFeaturePoints(data, f1);
	double enemyPoints = futureConnectionGetFeaturePoints(data, f2);

	if (enemyPoints < (toEmpty == toFeature ? 7.0 : 5.0)) {
//            System.err.println("too small penalty: " + enemyPoints);
		return  -0.05; //small penalty
	}

	double coef = toEmpty != toFeature ? 0.7 : 0.4; //corner / straight connection

//        System.err.println("@@@ @@@ " + myPower + "/" + myPoints + " vs " + bestEnemy + "/" + enemyPoints);

	if (myPower == bestEnemy) {
		return coef * (enemyPoints - myPoints) * chance;
	}
	if (myPower > bestEnemy) {
		return coef * enemyPoints * chance;

	}
	return -myPoints * chance; //no coef here

}

double jcz::JCZPlayer::futureConnectionGetFeaturePoints(jcz::JCZPlayer::RankData & data, const Node * feature) {
	if (feature->getTerrain() == Field) {
		return feature->getScore();
	} else {
		return getUnfinishedCompletablePoints(data, feature);
	}
}

void jcz::JCZPlayer::funtureConnectionSumPower(uchar & myPower, uchar & bestEnemy, const Node * f1, const Node * f2)
{
	bestEnemy = 0;
	for (int p = 0; p < (int)playerCount; ++p)
	{
		uchar const sum = (uchar)(f1->getPlayerMeeples(p) + f2->getPlayerMeeples(p));
		if (p == player)
			myPower = sum;
		else
		{
			if (sum > bestEnemy)
				bestEnemy = sum;
		}
	}
}

double jcz::JCZPlayer::futureConnectionRateCrossing(const Node * f1, const Node * f2) {
	uchar bestEnemy = 0;
	uchar myPower = 0;
	for (int p = 0; p < (int)playerCount; ++p)
	{
		uchar const m = f2->getPlayerMeeples(p);
		if (p == player)
		{
			myPower = m;
		}
		else
		{
			if (m > bestEnemy)
				bestEnemy = m;
		}
	}

	if (!f1->isOccupied()) {
		if (bestEnemy > myPower) {
			return 0.2;
		} else if (myPower > 0) {
			return -2.5;
		}
	} else {
		if (bestEnemy > myPower) {
			return -0.1;
		} else if (myPower > 0) {
			return -0.5;
		} else {
			return -0.5;
		}
	}
	return 0;
}

double jcz::JCZPlayer::rankConvexity(const jcz::JCZPlayer::RankData & data) {
	Board const * board = data.game->getBoard();
	int surrounding = 0;
	for (QPoint const & d : ADJACENT_AND_DIAGONAL)
		if (board->getTile(data.placement + d))
			++surrounding;
	return 0.001 * surrounding;
}
