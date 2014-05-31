#include "util.h"
#include "game.h"
#include "player/randomplayer.h"

Util::OffsetArray<qreal> Util::Math::lnTable = OffsetArray<qreal>();
Util::Math const Util::Math::instance = Util::Math();

void Util::syncGamesFast(const Game & from, Game & to)
{
	auto const & history = from.getMoveHistory();
	for (size_t i = to.getMoveHistory().size(), s = history.size(); i < s; ++i)
		to.simStep(history[i]);
}

void Util::syncGames(const Game & from, Game & to)
{
	auto const & fromHistory = from.getMoveHistory();
	auto & toHistory = to.getMoveHistory();

	while (toHistory.size() > fromHistory.size())
		to.simUndo();

	size_t offset = toHistory.size();
	for (size_t i = 0; i < toHistory.size(); ++i)
	{
		if (fromHistory[i] != toHistory[i])
		{
			offset = i;
			break;
		}
	}
	for (size_t i = offset; i < toHistory.size(); ++i)
		to.simUndo();

	syncGamesFast(from, to);
}

void Util::setupNewGame(const Game & from, Game & to, jcz::TileFactory * tileFactory)
{
	to.clearPlayers();
	for (uint i = 0; i < from.getPlayerCount(); ++i)
		to.addPlayer(&RandomPlayer::instance);
	to.newGame(from.getTileSets(), tileFactory, from.getMoveHistory());
}
