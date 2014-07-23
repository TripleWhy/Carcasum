/*
	This file is part of Carcasum.

	Carcasum is free software: you can redistribute it and/or modify
	it under the terms of the GNU Affero General Public License as published by
	the Free Software Foundation, either version 3 of the License, or
	(at your option) any later version.

	Carcasum is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU Affero General Public License for more details.

	You should have received a copy of the GNU Affero General Public License
	along with Carcasum.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "nexttileprovider.h"
#include "game.h"

int RandomNextTileProvider::nextTile(const Game * game)
{
	return r.nextInt(game->getTileCount());
}


HistoryProvider::HistoryProvider(NextTileProvider * ntp, const std::vector<MoveHistoryEntry> & history, size_t offset)
    : ntp(ntp),
      history(history),
      offset(offset)
{
}

void HistoryProvider::setData(const std::vector<MoveHistoryEntry> & history, size_t offset)
{
	HistoryProvider::history = history;
	HistoryProvider::offset = offset;
}

int HistoryProvider::nextTile(const Game * game)
{
	if (offset >= history.size())
		return ntp->nextTile(game);
	return history[offset++].tileIndex;
}
