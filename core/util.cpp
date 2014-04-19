#include "util.h"
#include "game.h"

Util::OffsetArray<qreal> Util::Math::lnTable = OffsetArray<qreal>();

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


Util::OffsetArray<qreal> Util::getUtilityMap(Game const * game)
{
	static OffsetArray<qreal> utilityMaps[MAX_PLAYERS];
	static int upperScoreBound = 0;

	const int playerCount = game->getPlayerCount();
	int const usb = game->getUpperScoreBound();
	if (usb != upperScoreBound)
	{
		//TODO Memory never gets deleted.
		for (int i = 0; i < MAX_PLAYERS; ++i)
			utilityMaps[i] = OffsetArray<qreal>();
		upperScoreBound = usb;
	}


	OffsetArray<qreal> & utilityMap = utilityMaps[playerCount];
	if (!utilityMap)
	{
		int size, offset;
		auto m = Util::newUtilityComplexNormalizationTable(playerCount, usb, size, offset);
		utilityMap = OffsetArray<qreal>(m, size, offset);
	}
	return utilityMap;
}
