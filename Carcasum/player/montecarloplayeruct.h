#ifndef MONTECARLOPLAYERUCT_H
#define MONTECARLOPLAYERUCT_H

#include "static.h"
#include "playouts.h"
#include "utilities.h"
#include "core/game.h"
#include "core/player.h"
#include "jcz/tilefactory.h"

template<class UtilityProvider = Utilities::ComplexUtilityNormalized, class Playout = Playouts::RandomPlayout>
class MonteCarloPlayerUCT : public Player
{
	constexpr static qreal Cp = 0.5;

#ifndef TIMEOUT
 #ifdef QT_NO_DEBUG
	static int const N = 100;
 #else
	static int const N = 20;
 #endif
#endif

	typedef typename UtilityProvider::RewardType RewardType;

private:
	Game const * game = 0;
	Game * simGame = 0;
	jcz::TileFactory * tileFactory;
	MeepleMove meepleMove;
	static Util::Math const & math;

	QString typeName;
	Playout playoutPolicy = Playout();
	UtilityProvider utilityProvider = UtilityProvider();
	const int M;
	const bool useTimeout;

//public:
//	int min = std::numeric_limits<int>::max();
//	int max = std::numeric_limits<int>::min();

public:
#ifdef TIMEOUT
	constexpr MonteCarloPlayerUCT(jcz::TileFactory * tileFactory, int m = TIMEOUT, bool mIsTimeout = true)
#else
	constexpr MonteCarloPlayerUCT(jcz::TileFactory * tileFactory, int m = 5000, bool mIsTimeout = true)
#endif
		: tileFactory(tileFactory),
	      typeName(QString("MonteCarloPlayerUCT<%1, %2>(m=%4, mIsTimeout=%5, Cp=%6)").arg(UtilityProvider::name).arg(Playout::name).arg(m).arg(mIsTimeout).arg(Cp)),
	      M(m),
		  useTimeout(mIsTimeout)
	{
	}

	virtual void newGame(int player, Game const * g);
	virtual void playerMoved(int player, Tile const * tile, MoveHistoryEntry const & move);
	virtual TileMove getTileMove(int player, Tile const * tile, MoveHistoryEntry const & move, TileMovesType const & placements);
	virtual MeepleMove getMeepleMove(int player, Tile const * tile, MoveHistoryEntry const & move, MeepleMovesType const & possible);
	virtual void endGame();
	virtual QString getTypeName() const { return typeName; }
	virtual Player * clone() const;

private:
	int playout();
	void unplayout(int steps);

	inline RewardType utility(int const * scores, int const playerCount, int const myIndex, Game * const g)
	{
		return utilityProvider.utility(scores, playerCount, myIndex, g);
	}

	inline int chooseTileMove(TileMovesType const & possible, uint N0, typename VarLengthArrayWrapper<RewardType, 128>::type const & Q1, VarLengthArrayWrapper<uint, 128>::type const & N1)
	{
		Q_ASSERT(possible.size() > 0);
		int const size = possible.size();
		qreal max = -std::numeric_limits<qreal>::infinity();
		int best = -1;
		for (int i = 0; i < size; ++i)
		{
			qreal val = (qreal(Q1[i]) / qreal(N1[i])) + Cp * math.sqrt( math.ln(N0) / N1[i] );
			if (val > max)
			{
				max = val;
				best = i;
			}
		}

		Q_ASSERT(best >= 0);
		return best;
	}

	inline int chooseMeepleMove(MeepleMovesType const & possible, uint N0, typename VarLengthArrayWrapper<RewardType, 16>::type const & Q1, VarLengthArrayWrapper<uint, 16>::type const & N1)
	{
		Q_ASSERT(possible.size() > 0);
		int const size = possible.size();
		qreal max = -std::numeric_limits<qreal>::infinity();
		int best = -1;
		for (int i = 0; i < size; ++i)
		{
			qreal val = (qreal(Q1[i]) / qreal(N1[i])) + Cp * math.sqrt( math.ln(N0) / N1[i] );
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

#include "montecarloplayeruct.tpp"

#endif // MONTECARLOPLAYERUCT_H
