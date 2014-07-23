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

#include "location.h"

#include <QStringList>

QMap<int, jcz::Location> jcz::Location::instances = QMap<int, jcz::Location>();


const jcz::Location jcz::Location::null = jcz::Location();
const jcz::Location jcz::Location::N = jcz::Location("N", 3 << 8);
const jcz::Location jcz::Location::W = jcz::Location("W", 192 << 8);
const jcz::Location jcz::Location::S = jcz::Location("S", 48 << 8);
const jcz::Location jcz::Location::E = jcz::Location("E", 12 << 8);

const jcz::Location jcz::Location::NW = jcz::Location("NW", 195 << 8);
const jcz::Location jcz::Location::SW = jcz::Location("SW", 240 << 8);
const jcz::Location jcz::Location::SE = jcz::Location("SE", 60 << 8);
const jcz::Location jcz::Location::NE = jcz::Location("NE", 15 << 8);

const jcz::Location jcz::Location::WE = jcz::Location("WE", 204 << 8);
const jcz::Location jcz::Location::NS = jcz::Location("NS", 51 << 8);
const jcz::Location jcz::Location::NWSE = jcz::Location("NWSE", 255 << 8);

const jcz::Location jcz::Location::_N = jcz::Location("_N", 252 << 8);
const jcz::Location jcz::Location::_W = jcz::Location("_W", 63 << 8);
const jcz::Location jcz::Location::_S = jcz::Location("_S", 207 << 8);
const jcz::Location jcz::Location::_E = jcz::Location("_E", 243 << 8);

const jcz::Location jcz::Location::CLOISTER = jcz::Location("CLOISTER", 1 << 16 );
const jcz::Location jcz::Location::TOWER = jcz::Location("TOWER", 1 << 17);
const jcz::Location jcz::Location::PRISON = jcz::Location("PRISON", 1 << 18);
const jcz::Location jcz::Location::FLIER = jcz::Location("FLIER", 1 << 19);

const jcz::Location jcz::Location::NL = jcz::Location("NL", 1);
const jcz::Location jcz::Location::NR = jcz::Location("NR", 2);
const jcz::Location jcz::Location::EL = jcz::Location("EL", 4);
const jcz::Location jcz::Location::ER = jcz::Location("ER", 8);
const jcz::Location jcz::Location::SL = jcz::Location("SL", 16);
const jcz::Location jcz::Location::SR = jcz::Location("SR", 32);
const jcz::Location jcz::Location::WL = jcz::Location("WL", 64);
const jcz::Location jcz::Location::WR = jcz::Location("WR", 128);
const jcz::Location jcz::Location::INNER_FARM = jcz::Location("INNER_FARM", 0);


const jcz::Location jcz::Location::SIDES[4] = {N, E, S, W};
const jcz::Location jcz::Location::DIAGONAL_SIDES[4] = {NE, SE, SW, NW};
const jcz::Location jcz::Location::FARM_SIDES[8] = {NL, NR, EL, ER, SL, SR, WL, WR};


jcz::Location jcz::Location::next() {
	return shift(2);
}

jcz::Location jcz::Location::prev() {
	return shift(6);
}

jcz::Location jcz::Location::rev() {
	// odd bits shift by 5, even by 3;
	int mLo = mask & 255;
	mLo = ((mLo & 85) << 5) | ((mLo & 170) << 3);
	mLo = (mLo | (mLo >> 8)) & 255;

	int mHi =  (mask & 65280) >> 8;
	mHi = ((mHi & 85) << 5) | ((mHi & 170) << 3);
	mHi = (mHi | (mHi >> 8)) & 255;

	return create((mask & ~65535) | (mHi << 8) | mLo);
}

jcz::Location jcz::Location::shift(int i) const {
	int mLo = (mask & 255) << i;
	mLo = (mLo | mLo >> 8) & 255;

	int mHi = (mask & 65280) << i;
	mHi = (mHi | mHi >> 8) & 65280;

	return create((mask & ~65535) | mHi | mLo);
}

jcz::Location jcz::Location::rotateCCW(signed char rot) const {
	return shift((rot*6)%8);
}

jcz::Location jcz::Location::rotateCW(signed char rot) const {
	return shift(rot*2);
}

jcz::Location jcz::Location::getLeftFarm() const {
	Q_ASSERT (isEdgeLocation());
	return create((mask >> 8) & 85);
}

jcz::Location jcz::Location::getRightFarm() const {
	Q_ASSERT(isEdgeLocation());
	return create((mask >> 8) & 170);
}

bool jcz::Location::isPartOf(jcz::Location const & d) const {
	if (mask == 0) return this == &d;
	return ((mask ^ d.mask) & mask) == 0;
}

QString jcz::Location::toString() const {
	if (!name.isEmpty()) return name;
	QString str;
	for (Location atom : FARM_SIDES) {
		if (!intersect(atom).isNull()) {
			if (str.length() > 0) str += ".";
			str += atom.toString();
		}
	}
	return str;
}

jcz::Location jcz::Location::unite(const jcz::Location & d) const {
	if (d.isNull()) return *this;
//	Q_ASSERT( !isSpecialLocation() && !(isEdgeLocation() ^ d.isEdgeLocation()) & !(isFarmLocation() ^ d.isFarmLocation()) : "union("+this+','+d+')' );
	return create(mask | d.mask);
}

jcz::Location jcz::Location::substract(const Location & d) const {
	if (d == null) return *this;
//	assert !isSpecialLocation() && !(isEdgeLocation() ^ d.isEdgeLocation()) & !(isFarmLocation() ^ d.isFarmLocation()) : "substract("+this+','+d+')';
	return create((~(mask & d.mask)) & mask);
}

jcz::Location jcz::Location::intersect(const Location & d) const {
	if (d == null || (mask & d.mask) == 0) return null;
//	assert !isSpecialLocation() && !(isEdgeLocation() ^ d.isEdgeLocation()) & !(isFarmLocation() ^ d.isFarmLocation()) : "interasect("+this+','+d+')';
	return create(mask & d.mask);
}

jcz::Location jcz::Location::valueOf(QString const & name) {
	Location value = null;
	for (QString const & part : name.split('.')) {
		Location item = null;
		for (Location const & l : Location::instances.values())
		{
			if (l.name == part)
			{
				item = l;
				break;
			}
		}
//		Q_ASSERT(!item.isNull());
		value = item.unite(value);
	}
//	Q_ASSERT(!value.isNull());
	return value;
}

signed char jcz::Location::getRotationOf(const Location & loc) const {
	for (signed char r = 0; r < 4; ++r) {
		if (operator ==(loc.rotateCW(r))) return r;
	}
	return -1;
}

bool jcz::Location::isRotationOf(const Location & loc)  const {
	return getRotationOf(loc) != 1;
}

bool jcz::Location::isFarmLocation() const {
	return (*this == INNER_FARM) || (mask & 255) > 0;
}

bool jcz::Location::isEdgeLocation() const {
	return (mask & 65280) > 0;
}

bool jcz::Location::isSpecialLocation() const {
	return (mask & ~65535) > 0;
}
