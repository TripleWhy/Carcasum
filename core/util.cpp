#include "util.h"
#include "game.h"

void Util::syncGamesFast(const Game & from, Game & to)
{
	auto const & history = from.getMoveHistory();
	for (size_t i = to.getMoveHistory().size(); i < history.size(); ++i)
		to.simStep(history[i]);
}

void Util::syncGames(const Game & from, Game & to)
{
	auto const & fromHistory = from.getMoveHistory();
	auto & toHistory = to.getMoveHistory();

	while (toHistory.size() > fromHistory.size())
		to.undo();

	int offset = toHistory.size();
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
