#include "util.h"
#include "game.h"

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
		to.undo();

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
		to.undo();

	syncGamesFast(from, to);
}
