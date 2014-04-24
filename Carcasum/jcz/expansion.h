#ifndef EXPANSION_H
#define EXPANSION_H

#include <QString>
#include "core/tile.h"

namespace jcz
{

enum Expansion
{
	//Basic sets
	BASIC = 1 << 0,
	WHEEL_OF_FORTUNE = 1 << 1, // Not implemented in JCZ

	//Winter branch
	WINTER = 1 << 2,
	GINGERBREAD_MAN = 1 << 3, // Not implemented in JCZ

	//Big expansions
	INNS_AND_CATHEDRALS = 1 << 4,
	TRADERS_AND_BUILDERS = 1 << 5,
	PRINCESS_AND_DRAGON = 1 << 6,
	TOWER = 1 << 7,
	ABBEY_AND_MAYOR = 1 << 8,
	CATAPULT = 1 << 9,
	BRIDGES_CASTLES_AND_BAZAARS = 1 << 10,
	KING_AND_ROBBER_BARON = 1 << 11,

	//Small expansion
	RIVER = 1 << 12,
	RIVER_II = 1 << 13,
	CATHARS = 1 << 14,
	BESIEGERS = 1 << 15,
	COUNT = 1 << 16,
	GQ11 = 1 << 17,
	CULT = 1 << 18,
	TUNNEL = 1 << 19,
	CORN_CIRCLES = 1 << 20,
	PLAGUE = 1 << 21,
	PHANTOM = 1 << 22,
	FESTIVAL = 1 << 23,
	HOUSES = 1 << 24,
	WIND_ROSE = 1 << 25,

	//minis expansion line
	FLIER = 1 << 26,
	MESSAGES = 1 << 27, // Not implemented in JCZ
	FERRIES = 1 << 28, // Not implemented in JCZ
	GOLDMINES = 1 << 29, // Not implemented in JCZ
	MAGE_WITCH = 1 << 30, // Not implemented in JCZ
	ROBBER = 1 << 31, // Not implemented in JCZ
	CORN_CIRCLES_II = (qint64)1 << 32,

	//promo/one tile expansions
	SCHOOL = (qint64)1 << 33, // Not implemented in JCZ
	LA_PORXADA = (qint64)1 << 34, // Not implemented in JCZ
};

class Expansions
{
	friend Expansions operator|(Expansions e1, Expansions e2);
private:
	quint64 values;

private:
public:
	static QString getCode(Expansion expansion);
	static QString getName(Expansion expansion);
	static Expansion fromTileSet(Tile::TileSet set);

	Q_DECL_CONSTEXPR inline Expansions() : values(0) {}
	Q_DECL_CONSTEXPR inline Expansions(Expansion f) : values(quint64(f)) {}
	Q_DECL_CONSTEXPR inline Expansions(quint64 f) : values(f) {}

	inline Expansions &operator&=(quint64 mask) { values &= mask; return *this; }
	inline Expansions &operator&=(uint mask) { values &= mask; return *this; }
	inline Expansions &operator&=(Expansion mask) { values &= quint64(mask); return *this; }
	inline Expansions &operator|=(Expansions f) { values |= f.values; return *this; }
	inline Expansions &operator|=(Expansion f) { values |= quint64(f); return *this; }
	inline Expansions &operator^=(Expansions f) { values ^= f.values; return *this; }
	inline Expansions &operator^=(Expansion f) { values ^= quint64(f); return *this; }

	Q_DECL_CONSTEXPR  inline operator quint64() const { return values; }

	Q_DECL_CONSTEXPR inline Expansions operator|(Expansions f) const { return Expansions(quint64(values | f.values)); }
	Q_DECL_CONSTEXPR inline Expansions operator|(Expansion f) const { return Expansions(quint64(values | quint64(f))); }
	Q_DECL_CONSTEXPR inline Expansions operator^(Expansions f) const { return Expansions(quint64(values ^ f.values)); }
	Q_DECL_CONSTEXPR inline Expansions operator^(Expansion f) const { return Expansions(quint64(values ^ quint64(f))); }
	Q_DECL_CONSTEXPR inline Expansions operator&(quint64 mask) const { return Expansions(quint64(values & mask)); }
	Q_DECL_CONSTEXPR inline Expansions operator&(uint mask) const { return Expansions(quint64(values & mask)); }
	Q_DECL_CONSTEXPR inline Expansions operator&(Expansion f) const { return Expansions(quint64(values & quint64(f))); }
	Q_DECL_CONSTEXPR inline Expansions operator~() const { return Expansions(quint64(~values)); }

	Q_DECL_CONSTEXPR inline bool operator!() const { return !values; }

	Q_DECL_CONSTEXPR inline bool testFlag(Expansion f) const { return (values & quint64(f)) == quint64(f) && (quint64(f) != 0 || values == quint64(f) ); }
};

}

#endif // EXPANSION_H
