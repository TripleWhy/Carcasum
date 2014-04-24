#include "expansion.h"

QString jcz::Expansions::getCode(jcz::Expansion expansion)
{
	switch (expansion)
	{
		case BASIC:
			return "BA";
		case WHEEL_OF_FORTUNE:
			return "WF";
		case WINTER:
			return "WI";
		case GINGERBREAD_MAN:
			return "GM";
		case INNS_AND_CATHEDRALS:
			return "IC";
		case TRADERS_AND_BUILDERS:
			return "TB";
		case PRINCESS_AND_DRAGON:
			return "DG";
		case TOWER:
			return "TO";
		case ABBEY_AND_MAYOR:
			return "AM";
		case CATAPULT:
			return "CA";
		case BRIDGES_CASTLES_AND_BAZAARS:
			return "BB";
		case KING_AND_ROBBER_BARON:
			return "KR";
		case RIVER:
			return "R1";
		case RIVER_II:
			return "R2";
		case CATHARS:
			return "SI";
		case BESIEGERS:
			return "BE";
		case COUNT:
			return "CO";
		case GQ11:
			return "GQ";
		case CULT:
			return "CU";
		case TUNNEL:
			return "TU";
		case CORN_CIRCLES:
			return "CC";
		case PLAGUE:
			return "PL";
		case PHANTOM:
			return "PH";
		case FESTIVAL:
			return "FE";
		case HOUSES:
			return "LB";
		case WIND_ROSE:
			return "WR";
		case FLIER:
			return "FL";
		case MESSAGES:
			return "ME";
		case FERRIES:
			return "FR";
		case GOLDMINES:
			return "GO";
		case MAGE_WITCH:
			return "MW";
		case ROBBER:
			return "RO";
		case CORN_CIRCLES_II:
			return "C2";
		case SCHOOL:
			return "SC";
		case LA_PORXADA:
			return "PX";
	}
	return QString();
}

QString jcz::Expansions::getName(jcz::Expansion expansion)
{
	switch (expansion)
	{
		case BASIC:
			return "BASIC";
		case WHEEL_OF_FORTUNE:
			return "WHEEL_OF_FORTUNE";
		case WINTER:
			return "WINTER";
		case GINGERBREAD_MAN:
			return "GINGERBREAD_MAN";
		case INNS_AND_CATHEDRALS:
			return "INNS_AND_CATHEDRALS";
		case TRADERS_AND_BUILDERS:
			return "TRADERS_AND_BUILDERS";
		case PRINCESS_AND_DRAGON:
			return "PRINCESS_AND_DRAGON";
		case TOWER:
			return "TOWER";
		case ABBEY_AND_MAYOR:
			return "ABBEY_AND_MAYOR";
		case CATAPULT:
			return "CATAPULT";
		case BRIDGES_CASTLES_AND_BAZAARS:
			return "BRIDGES_CASTLES_AND_BAZAARS";
		case KING_AND_ROBBER_BARON:
			return "KING_AND_ROBBER_BARON";
		case RIVER:
			return "RIVER";
		case RIVER_II:
			return "RIVER_II";
		case CATHARS:
			return "CATHARS";
		case BESIEGERS:
			return "BESIEGERS";
		case COUNT:
			return "COUNT";
		case GQ11:
			return "GQ11";
		case CULT:
			return "CULT";
		case TUNNEL:
			return "TUNNEL";
		case CORN_CIRCLES:
			return "CORN_CIRCLES";
		case PLAGUE:
			return "PLAGUE";
		case PHANTOM:
			return "PHANTOM";
		case FESTIVAL:
			return "FESTIVAL";
		case HOUSES:
			return "HOUSES";
		case WIND_ROSE:
			return "WIND_ROSE";
		case FLIER:
			return "FLIER";
		case MESSAGES:
			return "MESSAGES";
		case FERRIES:
			return "FERRIES";
		case GOLDMINES:
			return "GOLDMINES";
		case MAGE_WITCH:
			return "MAGE_WITCH";
		case ROBBER:
			return "ROBBER";
		case CORN_CIRCLES_II:
			return "CORN_CIRCLES_II";
		case SCHOOL:
			return "SCHOOL";
		case LA_PORXADA:
			return "LA_PORXADA";
	}
	return QString();
}

jcz::Expansion jcz::Expansions::fromTileSet(Tile::TileSet set)
{
	switch (set)
	{
		case Tile::BaseGame:
			return BASIC;
	}
	return BASIC;
}
