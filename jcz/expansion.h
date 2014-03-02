#ifndef EXPANSION_H
#define EXPANSION_H

#include <QString>

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
	Expansions(quint64 v) : values(v) {}
public:
	Expansions(Expansion e) : values(e) {}

	static QString getCode(Expansion expansion);
	static QString getName(Expansion expansion);
};
inline Expansions operator|(Expansions e1, Expansions e2)
{
	return Expansions(e1.values | e2.values);
}

}

#endif // EXPANSION_H
