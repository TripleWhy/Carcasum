#include "specialplayers.h"
#include "player/randomplayer.h"
#include "simpleplayer3.h"

#define EG_T template<int RndPercent, typename Pl>
#define EG_TU <RndPercent, Pl>

EG_T
void EGreedyPlayer EG_TU::newGame(int player, const Game * g)
{
	m_player.newGame(player, g);
}

EG_T
void EGreedyPlayer EG_TU::playerMoved(int player, const Tile * tile, const MoveHistoryEntry & move)
{
	m_player.playerMoved(player, tile, move);
}

EG_T
void EGreedyPlayer EG_TU::undoneMove(const MoveHistoryEntry & move)
{
	m_player.undoneMove(move);
}

EG_T
TileMove EGreedyPlayer EG_TU::getTileMove(int player, const Tile * tile, const MoveHistoryEntry & move, const TileMovesType & possible)
{
	if (RndPercent >= 0 && (RndPercent >= 100 || (r.nextInt(100) < RndPercent)) )
		return RandomPlayer::instance.getTileMove(player, tile, move, possible);
	else
		return m_player.getTileMove(player, tile, move, possible);
}

EG_T
MeepleMove EGreedyPlayer EG_TU::getMeepleMove(int player, const Tile * tile, const MoveHistoryEntry & move, const MeepleMovesType & possible)
{
	if (RndPercent >= 0 && (RndPercent >= 100 || (r.nextInt(100) < RndPercent)) )
		return RandomPlayer::instance.getMeepleMove(player, tile, move, possible);
	else
		return m_player.getMeepleMove(player, tile, move, possible);
}

EG_T
void EGreedyPlayer EG_TU::endGame()
{
	m_player.endGame();
}

EG_T
QString EGreedyPlayer EG_TU::getTypeName() const
{
	return name;
}

EG_T
Player * EGreedyPlayer EG_TU::clone() const
{
	return new EGreedyPlayer();
}



void RouletteWheelPlayer::newGame(int /*player*/, const Game * game)
{
	RouletteWheelPlayer::game = game;
}

void RouletteWheelPlayer::playerMoved(int /*player*/, const Tile * /*tile*/, const MoveHistoryEntry & /*move*/)
{
}

void RouletteWheelPlayer::undoneMove(const MoveHistoryEntry & /*move*/)
{
}

TileMove RouletteWheelPlayer::getTileMove(int player, const Tile * tile, const MoveHistoryEntry & /*move*/, const TileMovesType & possible)
{
	int sum;
	SimplePlayer3::RatingsEType && ratings = SimplePlayer3::rateAllExpanded(sum, game, player, tile, possible);

	int rnd = r.nextInt(sum);
	for (auto const & rating : ratings)
	{
		Q_ASSERT(rating.second >= 0);
		rnd -= rating.second;
		if (rnd < 0)
		{
			meepleMove = rating.first.meepleMove;
			return rating.first.tileMove;
		}
	}
	Q_ASSERT(false);
	return ratings.back().first.tileMove;
}

MeepleMove RouletteWheelPlayer::getMeepleMove(int player, const Tile * tile, const MoveHistoryEntry & move, const MeepleMovesType & possible)
{
#if SIMPLE_PLAYER3_RULE_FIELD
	if (std::find(possible.cbegin(), possible.cend(), meepleMove) == possible.cend())	// See SimplePlayer3
		return RandomPlayer::instance.getMeepleMove(player, tile, move, possible);
#else
	Q_ASSERT(std::find(possible.cbegin(), possible.cend(), meepleMove) != possible.cend());;
#endif
	return meepleMove;
}

void RouletteWheelPlayer::endGame()
{
}

QString RouletteWheelPlayer::getTypeName() const
{
	return "RouletteWheelPlayer";
}

Player *RouletteWheelPlayer::clone() const
{
	return new RouletteWheelPlayer();
}



void RouletteWheelPlayer2::newGame(int /*player*/, const Game * game)
{
	RouletteWheelPlayer2::game = game;
}

void RouletteWheelPlayer2::playerMoved(int /*player*/, const Tile * /*tile*/, const MoveHistoryEntry & /*move*/)
{
}

void RouletteWheelPlayer2::undoneMove(const MoveHistoryEntry & /*move*/)
{
}

TileMove RouletteWheelPlayer2::getTileMove(int player, const Tile * tile, const MoveHistoryEntry & /*move*/, const TileMovesType & possible)
{
	int sum;
	SimplePlayer3::RatingsNType && ratings = SimplePlayer3::rateAllNested(sum, game, player, tile, possible);

	int rnd = r.nextInt(sum);
	for (auto const & rating : ratings)
	{
		Q_ASSERT(rating.tileRating >= 0);
		rnd -= rating.tileRating;
		if (rnd < 0)
		{
			meepleSum = rating.meepleSum;
			meepleRatings = std::move(rating.meepleRatings);
			return rating.tileMove;
		}
	}
	Q_ASSERT(false);
	meepleRatings = std::move(ratings.back().meepleRatings);
	return ratings.back().tileMove;
}

MeepleMove RouletteWheelPlayer2::getMeepleMove(int /*player*/, const Tile * /*tile*/, const MoveHistoryEntry & /*move*/, const MeepleMovesType & possible)
{
#if SIMPLE_PLAYER3_RULE_FIELD
	// remove invalid moves...
	for (auto it = meepleRatings.begin(); it != meepleRatings.end(); )
	{
		if (std::find(possible.cbegin(), possible.cend(), (*it).meepleMove) == possible.cend())
		{
			Q_ASSERT ((*it).meepleRating >= 0);
			meepleSum -= (*it).meepleRating;
			it = meepleRatings.erase(it);
		}
		else
			++it;
	}
#endif

	int rnd = r.nextInt(meepleSum);
	for (auto const & rating : meepleRatings)
	{
		Q_ASSERT(rating.meepleRating >= 0);
		rnd -= rating.meepleRating;
		if (rnd < 0)
			return rating.meepleMove;
	}
	Q_ASSERT(false);
	return meepleRatings.back().meepleMove;
}

void RouletteWheelPlayer2::endGame()
{
}

QString RouletteWheelPlayer2::getTypeName() const
{
	return "RouletteWheelPlayer2";
}

Player *RouletteWheelPlayer2::clone() const
{
	return new RouletteWheelPlayer2();
}
