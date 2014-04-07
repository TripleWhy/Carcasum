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

#if COUNT_PLAYOUTS
public:
	int playouts = 0;
#endif

private:
	Game const * game = 0;
	Game * simGame = 0;
	jcz::TileFactory * tileFactory;
	bool useComplexUtility;
	MeepleMove meepleMove;
	static RandomTable r;

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

	inline static int utilitySimple(int const * scores, int const playerCount, int const myIndex)
	{
		return Util::utilitySimple(scores, playerCount, myIndex);
	}

	inline static int utilityComplex(int const * scores, int const playerCount, int const myIndex)
	{
		return Util::utilityComplex(scores, playerCount, myIndex);
	}

	typedef int (*utilityFunctionType)(int const *, int const, int const);
	constexpr utilityFunctionType utilityFunction()
	{
		return useComplexUtility ? &utilityComplex : &utilitySimple;
	}

	inline int utility(int const * scores, int const playerCount, int const myIndex)
	{
//		return utilityComplex(scores, playerCount, myIndex);
		return useComplexUtility ? utilityComplex(scores, playerCount, myIndex) : utilitySimple(scores, playerCount, myIndex);
//		return utilityFunction()(scores, playerCount, myIndex);
	}

	inline int chooseTileMove(TileMovesType const & possible, int N0, VarLengthArrayWrapper<int, 128>::type const & Q1, VarLengthArrayWrapper<int, 128>::type const & N1)
	{
		Q_ASSERT(possible.size() > 0);
		int const size = possible.size();
		qreal max = -std::numeric_limits<qreal>::infinity();
		int best = -1;
		for (int i = 0; i < size; ++i)
		{
			qreal val = (Q1[i] / qreal(N1[i])) + Cp * Util::mysqrt( Util::ln(N0) / N1[i] );
			if (val > max)
			{
				max = val;
				best = i;
			}
		}

		Q_ASSERT(best >= 0);
		return best;
	}

	inline int chooseMeepleMove(MeepleMovesType const & possible, int N0, VarLengthArrayWrapper<int, 16>::type const & Q1, VarLengthArrayWrapper<int, 16>::type const & N1)
	{
		Q_ASSERT(possible.size() > 0);
		int const size = possible.size();
		qreal max = -std::numeric_limits<qreal>::infinity();
		int best = -1;
		for (int i = 0; i < size; ++i)
		{
			qreal val = (Q1[i] / qreal(N1[i])) + Cp * Util::mysqrt( Util::ln(N0) / N1[i] );
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
