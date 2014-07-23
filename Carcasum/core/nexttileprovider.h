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

#ifndef NEXTTILEPROVIDER_H
#define NEXTTILEPROVIDER_H

#include "static.h"
#include "random.h"

class Game;
struct MoveHistoryEntry;

class NextTileProvider
{
public:
	virtual int nextTile(Game const * game) = 0;
};

class RandomNextTileProvider : public NextTileProvider
{
private:
	DefaultRandom r;
public:
	virtual int nextTile(Game const * game);
};

class HistoryProvider : public NextTileProvider
{
private:
	NextTileProvider * ntp;
	std::vector<MoveHistoryEntry> history;
	size_t offset;

public:
	HistoryProvider(NextTileProvider * ntp, std::vector<MoveHistoryEntry> const & history = {}, size_t offset = 0);
	void setData(std::vector<MoveHistoryEntry> const & history, size_t offset);
	virtual int nextTile(Game const * game);
};

#endif // NEXTTILEPROVIDER_H
