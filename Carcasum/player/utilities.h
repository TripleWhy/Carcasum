#ifndef UTILITIES_H
#define UTILITIES_H

#include "static.h"
#include "core/util.h"
#include "core/game.h"
#include <string>

namespace Utilities
{

// Careful with these, they do not work for all utilities.
template<typename UtilityType>
static inline typename UtilityType::RewardType utilityUpperBound(int const playerCount, int const upperScoreBound, Game const * g)
{
	constexpr UtilityType u;
	int scores[MAX_PLAYERS] = { upperScoreBound };
	return u.utility(scores, playerCount, 0, g);
}

template<typename UtilityType>
static inline typename UtilityType::RewardType utilityLowerBound(int const playerCount, int const upperScoreBound, Game const * g)
{
	constexpr UtilityType u;
	int scores[MAX_PLAYERS] = { };
	for (int i = 1; i < playerCount; ++i)
		scores[i] = upperScoreBound;
	return u.utility(scores, playerCount, 0, g);
}
template<typename UtilityType>
static inline typename UtilityType::RewardType utilityUpperBound(UtilityType const & u, int const playerCount, int const upperScoreBound, Game const * g)
{
	int scores[MAX_PLAYERS] = { upperScoreBound };
	return u.utility(scores, playerCount, 0, g);
}

template<typename UtilityType>
static inline typename UtilityType::RewardType utilityLowerBound(UtilityType const & u, int const playerCount, int const upperScoreBound, Game const * g)
{
	int scores[MAX_PLAYERS] = { };
	for (int i = 1; i < playerCount; ++i)
		scores[i] = upperScoreBound;
	return u.utility(scores, playerCount, 0, g);
}

class SimpleUtility
{
public:
	constexpr static char const * name = "SimpleUtility";
	typedef int RewardType;
	typedef typename VarLengthArrayWrapper<RewardType, MAX_PLAYERS>::type RewardListType;
	inline void newGame(int /*player*/, Game const * /*g*/) {}
	RewardType utility(int const * scores, int const playerCount, int const myIndex, Game const * /*g*/) const
	{
		int best = std::numeric_limits<int>::min();
		int winner = -1;
		for (int i = 0; i < playerCount; ++i)
		{
			const int score = scores[i];
			if (score > best)
			{
				best = score;
				winner = i;
			}
			else if (score == best)
			{
				winner = -1;
			}
		}
		if (winner == myIndex)
			return 1;
		else if (winner == -1 && best == scores[myIndex])
			return 0;
		else
			return -1;
	}
	RewardListType utilities(const int * scores, const int playerCount, Game const * /*g*/) const
	{
		RewardListType reward(playerCount);
		int max = std::numeric_limits<int>::min();
		int winner = -1;
		for (int i = 0; i < playerCount; ++i)
		{
			int s = scores [i];
			if (s > max)
			{
				max = s;
				winner = i;
			}
			else if (s == max)
			{
				winner = -1;
			}
		}

		for (int i = 0; i < playerCount; ++i)
		{
			if (i == winner)
				reward[i] = 1;
			else if (winner == -1 && scores[i] == max)
				reward[i] = 0;
			else
				reward[i] = -1;
		}
		return reward;
	}
};

class SimpleUtilityF
{
public:
	constexpr static char const * name = "SimpleUtilityF";
	typedef qreal RewardType;
	typedef typename VarLengthArrayWrapper<RewardType, MAX_PLAYERS>::type RewardListType;
	inline void newGame(int /*player*/, Game const * /*g*/) {}
	RewardType utility(int const * scores, int const playerCount, int const myIndex, Game const * /*g*/) const
	{
		int best = std::numeric_limits<int>::min();
		int winner = -1;
		for (int i = 0; i < playerCount; ++i)
		{
			const int score = scores[i];
			if (score > best)
			{
				best = score;
				winner = i;
			}
			else if (score == best)
			{
				winner = -1;
			}
		}
		if (winner == myIndex)
			return 1;
		else if (winner == -1 && best == scores[myIndex])
			return 0;
		else
			return -1;
	}
	RewardListType utilities(const int * scores, const int playerCount, Game const * /*g*/) const
	{
		RewardListType reward(playerCount);
		int max = std::numeric_limits<int>::min();
		int winner = -1;
		for (int i = 0; i < playerCount; ++i)
		{
			int s = scores [i];
			if (s > max)
			{
				max = s;
				winner = i;
			}
			else if (s == max)
			{
				winner = -1;
			}
		}

		for (int i = 0; i < playerCount; ++i)
		{
			if (i == winner)
				reward[i] = 1;
			else if (winner == -1 && scores[i] == max)
				reward[i] = 0;
			else
				reward[i] = -1;
		}
		return reward;
	}
};

class SimpleUtilityNormalized
{
public:
	constexpr static char const * name = "SimpleUtilityNormalized";
	typedef qreal RewardType;
	typedef typename VarLengthArrayWrapper<RewardType, MAX_PLAYERS>::type RewardListType;
	inline void newGame(int /*player*/, Game const * /*g*/) {}
	RewardType utility(int const * scores, int const playerCount, int const myIndex, Game const * /*g*/) const
	{
		int best = std::numeric_limits<int>::min();
		int winner = -1;
		for (int i = 0; i < playerCount; ++i)
		{
			const int score = scores[i];
			if (score > best)
			{
				best = score;
				winner = i;
			}
			else if (score == best)
			{
				winner = 0;
			}
		}
		if (winner == myIndex)
			return 1;
		else if (winner == -1 && best == scores[myIndex])
			return 0.5;
		else
			return 0;
	}
	RewardListType utilities(const int * scores, const int playerCount, Game const * /*g*/) const
	{
		RewardListType reward(playerCount);
		int max = std::numeric_limits<int>::min();
		int winner = -1;
		for (int i = 0; i < playerCount; ++i)
		{
			int s = scores [i];
			if (s > max)
			{
				max = s;
				winner = i;
			}
			else if (s == max)
			{
				winner = -1;
			}
		}

		for (int i = 0; i < playerCount; ++i)
		{
			if (i == winner)
				reward[i] = 1;
			else if (winner == -1 && scores[i] == max)
				reward[i] = 0.5;
			else
				reward[i] = 0;
		}
		return reward;
	}
};

class ComplexUtility
{
private:
	Util::OffsetArray<qreal> utilityMap;

public:
	constexpr static char const * name = "ComplexUtility";
	typedef int RewardType;
	typedef typename VarLengthArrayWrapper<RewardType, MAX_PLAYERS>::type RewardListType;

	inline void newGame(int /*player*/, Game const * /*g*/) {}

	RewardType utility(int const * scores, int const playerCount, int const myIndex, Game const * /*g*/) const
	{
		std::multimap<int, int> map;
		for (int i = 0; i < playerCount; ++i)
			map.insert(std::pair<int, int>(scores[i], i));

		int u = 0;
		int m = 0;
		int lastScore = -1;
		int index = 1;
		// last player first
		for (auto const & e : map)
		{
			if (e.first != lastScore)
			{
				int count = (int)map.count(e.first);
				m = index * index;
				for (int i = 1; i < count; ++i)
					m += (index+i)*(index+i);
				m /= count;
			}

			if (e.second == myIndex)
				u += m * e.first;
			else
				u -= m * e.first;
			lastScore = e.first;
			++index;
		}

		return u;
	}

	RewardListType utilities(const int * scores, const int playerCount, Game const * /*g*/) const
	{
		RewardListType reward(playerCount);

		std::multimap<int, int> map;
		for (int i = 0; i < playerCount; ++i)
			map.insert(std::pair<int, int>(scores[i], i));

		int u = 0;
		int m = 0;
		int lastScore = -1;
		int index = 1;
		// last player first
		for (auto const & e : map)
		{
			if (e.first != lastScore)
			{
				int count = (int)map.count(e.first);
				m = index * index;
				for (int i = 1; i < count; ++i)
					m += (index+i)*(index+i);
				m /= count;
			}

			u += (reward[e.second] = m * e.first);
			lastScore = e.first;
			++index;
		}

		for (int i = 0; i < playerCount; ++i)
		{
			reward[i] = 2*reward[i] - u;
		}

		return reward;
	}
};

class ComplexUtilityNormalized
{
public:
	constexpr static char const * name = "ComplexUtilityNormalized";
	typedef qreal RewardType;
	typedef typename VarLengthArrayWrapper<RewardType, MAX_PLAYERS>::type RewardListType;

private:
	ComplexUtility complex;
	int uBound = -1;
	int lBound = 0;
	int range = 0;
	Util::OffsetArray<qreal> utilityMap;

private:
	inline qreal * newUtilityComplexNormalizationTable(int const playerCount, int & size, int & offset)
	{
		int const range = uBound - lBound;
		qreal const A = (range * playerCount) / 300.0;
		qreal const B = (6 * playerCount) / qreal(range);

		size = range + 1;
		offset = -lBound;
		qreal * table = new qreal[size];
		for (int i = lBound; i <= uBound; ++i)
			table[i - lBound] = tanh(((i+A) * B) + 1) / 2;
		return table;
	}

public:
	~ComplexUtilityNormalized()
	{
		utilityMap.clear();
	}

	inline void newGame(int player, Game const * g)
	{
		complex.newGame(player, g);
		const int upperScoreBound = g->getUpperScoreBound();
		const int playerCount = g->getPlayerCount();
		int ub = utilityUpperBound(complex, playerCount, upperScoreBound, g);
		int lb = utilityLowerBound(complex, playerCount, upperScoreBound, g);
		if (uBound != ub || lBound != lb)
		{
			uBound = ub;
			lBound = lb;
			range = uBound - lBound;
			int size, offset;
			auto m = newUtilityComplexNormalizationTable(playerCount, size, offset);
			utilityMap.setData(m, size, offset);
		}
	}

	RewardType utility(int const * scores, int const playerCount, int const myIndex, Game const * g) const
	{
		int const u = complex.utility(scores, playerCount, myIndex, g);

		qreal const A = (range * playerCount) / 300.0;
		qreal const B = (6 * playerCount) / qreal(range);

		return tanh(((u+A) * B) + 1) / 2;
	}

	RewardListType utilities(const int * scores, const int playerCount, Game const * g) const
	{
		auto const & u = complex.utilities(scores, playerCount, g);
		RewardListType r(u.size());
		for (int i = 0; i < u.size(); ++i)
			r[i] = utilityMap[u[i]];
		return r;
	}
};

class HeydensUtility
{
public:
	constexpr static char const * name = "HeydensUtility";
	typedef int RewardType;
	typedef typename VarLengthArrayWrapper<RewardType, MAX_PLAYERS>::type RewardListType;
	inline void newGame(int /*player*/, Game const * /*g*/) {}
	RewardType utility(int const * scores, int const playerCount, int const myIndex, Game const * /*g*/) const
	{
		int u = 0;
		for (int i = 0; i < playerCount; ++i)
			if (i == myIndex)
				u += scores[i];
			else
				u -= scores[i];
		return u;
	}
	RewardListType utilities(const int * scores, const int playerCount, Game const * /*g*/) const
	{
		RewardListType reward(playerCount);
		int u = 0;
		for (int i = 0; i < playerCount; ++i)
				u += scores[i];
		for (int i = 0; i < playerCount; ++i)
			reward[i] = 2*scores[i] - u;
		return reward;
	}
};


template<typename Utility>
class EC
{
public:
	static QString const name;
	typedef typename Utility::RewardType RewardType;
	typedef typename Utility::RewardListType RewardListType;

private:
	Utility util;
	SimpleUtility simple;
	RewardType uBound = -1;
	RewardType lBound = 0;

public:
	inline void newGame(int player, Game const * g)
	{
		util.newGame(player, g);
		simple.newGame(player, g);

		const int upperScoreBound = g->getUpperScoreBound();
		const int playerCount = g->getPlayerCount();
		uBound = utilityUpperBound(util, playerCount, upperScoreBound, g);
		lBound = utilityLowerBound(util, playerCount, upperScoreBound, g);
	}

	RewardType utility(int const * scores, int const playerCount, int const myIndex, Game const * g) const
	{
		if (g->isTerminal())
		{
			switch (simple.utility(scores, playerCount, myIndex, g))
			{
				case -1:
					return lBound;
				case 0:
					return 0;
				case 1:
					return uBound;
			}
			Q_UNREACHABLE();
			return 0;
		}
		else
			return util.utility(scores, playerCount, myIndex, g);
	}

	RewardListType utilities(const int * scores, const int playerCount, Game const * g) const
	{
		if (g->isTerminal())
		{
			RewardListType reward(playerCount);
			auto const & r = simple.utilities(scores, playerCount, g);
			for (int i = 0; i < playerCount; ++i)
			{
				switch (r[i])
				{
					case -1:
						reward[i] = lBound;
						break;
					case 0:
						reward[i] = 0;
						break;
					case 1:
						reward[i] = uBound;
						break;
				}
			}
			return reward;
		}
		else
			return util.utilities(scores, playerCount, g);
	}
};
template<typename Utility>
QString const Utilities::EC<Utility>::name = QString("EC<%1>").arg(Utility::name);
typedef EC<ComplexUtility> ComplexUtilityEC;
typedef EC<HeydensUtility> HeydensUtilityEC;

class ComplexUtilityNormalizedEC
{
public:
	constexpr static char const * name = "ComplexUtilityNormalizedEC";
	typedef qreal RewardType;
	typedef typename VarLengthArrayWrapper<RewardType, MAX_PLAYERS>::type RewardListType;

private:
	ComplexUtilityNormalized complex;
	SimpleUtilityNormalized simple;

public:
	inline void newGame(int player, Game const * g)
	{
		complex.newGame(player, g);
		simple.newGame(player, g);
	}

	RewardType utility(int const * scores, int const playerCount, int const myIndex, Game const * g) const
	{
		if (g->isTerminal())
			return simple.utility(scores, playerCount, myIndex, g);
		else
			return complex.utility(scores, playerCount, myIndex, g);
	}
	RewardListType utilities(const int * scores, const int playerCount, Game const * g) const
	{
		if (g->isTerminal())
			return simple.utilities(scores, playerCount, g);
		else
			return complex.utilities(scores, playerCount, g);
	}
};

}

#endif // UTILITIES_H
