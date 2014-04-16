#ifndef MONTECARLOPLAYERUCT_H
#define MONTECARLOPLAYERUCT_H

#include "static.h"
#include "core/game.h"
#include "core/player.h"
#include "jcz/tilefactory.h"

class MonteCarloPlayerUCT : public Player
{
	constexpr static qreal Cp = 1;

#ifndef TIMEOUT
 #ifdef QT_NO_DEBUG
	static int const N = 100;
 #else
	static int const N = 20;
 #endif
#endif

	typedef Util::OffsetArray<qreal> UtilityMapType;

private:
	Game const * game = 0;
	Game * simGame = 0;
	jcz::TileFactory * tileFactory;
	bool useComplexUtility;
	MeepleMove meepleMove;
	UtilityMapType utilityMap;
	int utilityOffset = 0;
	static RandomTable r;
	static UtilityMapType utilityMaps[MAX_PLAYERS];
	static Util::Math const & math;

public:
	constexpr MonteCarloPlayerUCT(jcz::TileFactory * tileFactory, bool useComplexUtility = true)
		: tileFactory(tileFactory),
		  useComplexUtility(useComplexUtility)
	{
	}

	virtual void newGame(int player, Game const * g);
	virtual void playerMoved(int player, Tile const * tile, MoveHistoryEntry const & move);
	virtual TileMove getTileMove(int player, Tile const * tile, MoveHistoryEntry const & move, TileMovesType const & placements);
	virtual MeepleMove getMeepleMove(int player, Tile const * tile, MoveHistoryEntry const & move, MeepleMovesType const & possible);
	virtual void endGame();
	virtual char const * getTypeName() { return "MonteCarloPlayerUCT"; }

private:
	int playout();
	void unplayout(int steps);

	inline qreal utilitySimple(int const * scores, int const playerCount, int const myIndex)
	{
		return Util::utilitySimple(scores, playerCount, myIndex);
	}

	inline qreal utilityComplex(int const * scores, int const playerCount, int const myIndex)
	{
		int const u = Util::utilityComplex(scores, playerCount, myIndex);
		return utilityMap[u];
	}

	inline qreal utility(int const * scores, int const playerCount, int const myIndex)
	{
//		return utilityComplex(scores, playerCount, myIndex);
		return useComplexUtility ? utilityComplex(scores, playerCount, myIndex) : utilitySimple(scores, playerCount, myIndex);
//		return utilityFunction()(scores, playerCount, myIndex);
	}

	inline int chooseTileMove(TileMovesType const & possible, uint N0, VarLengthArrayWrapper<qreal, 128>::type const & Q1, VarLengthArrayWrapper<uint, 128>::type const & N1)
	{
		Q_ASSERT(possible.size() > 0);
		int const size = possible.size();
		qreal max = -std::numeric_limits<qreal>::infinity();
		int best = -1;
		for (int i = 0; i < size; ++i)
		{
			qreal val = (Q1[i] / qreal(N1[i])) + Cp * math.sqrt( math.ln(N0) / N1[i] );
			if (val > max)
			{
				max = val;
				best = i;
			}
		}

		Q_ASSERT(best >= 0);
		return best;
	}

	inline int chooseMeepleMove(MeepleMovesType const & possible, uint N0, VarLengthArrayWrapper<qreal, 16>::type const & Q1, VarLengthArrayWrapper<uint, 16>::type const & N1)
	{
		Q_ASSERT(possible.size() > 0);
		int const size = possible.size();
		qreal max = -std::numeric_limits<qreal>::infinity();
		int best = -1;
		for (int i = 0; i < size; ++i)
		{
			qreal val = (Q1[i] / qreal(N1[i])) + Cp * math.sqrt( math.ln(N0) / N1[i] );
			Q_ASSERT(val > -std::numeric_limits<qreal>::infinity());
			if (val > max)
			{
				max = val;
				best = i;
			}
		}

		Q_ASSERT(best >= 0);
		return best;
	}
};

#endif // MONTECARLOPLAYERUCT_H
