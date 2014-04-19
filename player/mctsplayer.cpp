#include "mctsplayer.h"

template<>
MCTSPlayer_Helper<true>::RewardListType MCTSPlayer_Helper<true>::utility(const int * scores, const int playerCount, Util::OffsetArray<qreal> const & utilityMap)
{
	auto const & u = Util::utilityComplexMulti(scores, playerCount);
	RewardListType r(u.size());
	for (int i = 0; i < u.size(); ++i)
		r[i] = (RewardType)(utilityMap[u[i]]);
	return r;
}

template<>
MCTSPlayer_Helper<false>::RewardListType MCTSPlayer_Helper<false>::utility(const int * scores, const int playerCount, Util::OffsetArray<qreal> const & )
{
	return Util::utilitySimpleMulti(scores, playerCount);
}
