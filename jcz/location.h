#ifndef LOCATION_H
#define LOCATION_H

// Directly imported code from jcz and translated to c++.
#include <QString>
#include <QMap>

namespace jcz
{

/**
 * bite order                     corresponding constants
 *
 *     0  1                               1  2
 *   7      2                          128     4
 *   6      3                          64      8
 *     5  4                              32  16
 */
class Location {
	friend int qHash(jcz::Location const & l);

private:
	QString name;
	int mask;

	static QMap<int, Location> instances;

	/**
	 * Obtains instance with given mask. For named location
	 * instance is unique.
	 * @param mask	bite mask of demand instance
	 */
public:
	static Location create(int mask) {
		if (instances.contains(mask))
			return instances[mask];
		//TODO prepare name here
		return Location(QString(), mask);
	}

private:
	Location(QString const & name, int mask)
		: name(name),
		  mask(mask)
	{
		instances.insert(mask, *this);
	}

public:
	Location() : Location(QString(), 0) {}

	/** North */
public:
	static const Location null;
	static const Location N;
	/** West */
	static const Location W;
	/** South */
	static const Location S;
	/** East */
	static const Location E;

	/** North-west */
	static const Location NW;
	/** South-west */
	static const Location SW;
	/** South-east */
	static const Location SE;
	/** North-east */
	static const Location NE;

	/** Horizontal location - W + E */
	static const Location WE;
	/** Vertical location -  N + S */
	static const Location NS;
	/** All edge locations */
	static const Location NWSE;

	/** Supplement to the north */
	static const Location _N;
	/** Supplement to the west  */
	static const Location _W;
	/** Supplement to the south */
	static const Location _S;
	/** Supplement to the east */
	static const Location _E;

	/** Cloister on tile */
	static const Location CLOISTER;
	/** Tower on tile */
	static const Location TOWER;
	/** Inprisoned follwer */
	static const Location PRISON;
	/** Flier location - follower can be placed here just for moment, before dice roll  */
	static const Location FLIER;

	// --- farm locations ---

	/** North left farm */
	static const Location NL;
	/** North right farm */
	static const Location NR;
	/** East left farm */
	static const Location EL;
	/** East right farm */
	static const Location ER;
	/** South left farm */
	static const Location SL;
	/** South right farm */
	static const Location SR;
	/** West left farm */
	static const Location WL;
	/** West right farm */
	static const Location WR;
	/** Center farm*/
	static const Location INNER_FARM;


public:
	static const Location SIDES[4];
	static const Location DIAGONAL_SIDES[4];
	static const Location FARM_SIDES[8];


public:
	bool operator==(Location const & obj) const {
//		if (this == &obj) return true;
		return mask == obj.mask;
	}

	/** Rotation about quarter circle clockwise */
	Location next();

	/** Rotation about quarter circle counter-clockwise */
	Location prev();

	/** Returns opposite location, mirrored by axis */
	Location rev();

	/**
	 * Bitwise mask rotation about given number of bites.
	 * @param i 	number of bites to rotate
	 * @return rotated location
	 */
private:
	Location shift(int i) const;

	/**
	 * Relative rotations in counter-clockwise location
	 * @param d how much rotate
	 * @return rotated location
	 */
public:
	Location rotateCCW(signed char rot) const;

	/**
	 * Relative rotations in clockwise location
	 */
	Location rotateCW(signed char rot) const;

//	static Location[] sides() {
//		return SIDES;
//	}

//	static Location[] sidesFarm() {
//		return FARM_SIDES;
//	}

//	static Location[] sidesDiagonal() {
//		return DIAGONAL_SIDES;
//	}

	Location getLeftFarm() const;

	Location getRightFarm() const;


	/** Checks if this is part of given location */
	bool isPartOf(const Location & d) const;

	QString toString() const;

	/** Merge two locations together */
	Location unite(Location const & d) const;

	/** Subtract given location from this */
	Location substract(Location const & d) const;

	Location intersect(Location const & d) const;

//	Location[] intersectMulti(Location[] locs) {
//		List<Location> result = new ArrayList<>();
//		for (Location loc: locs) {
//			Location i = this.intersect(loc);
//			if (i != null) {
//				result.add(i);
//			}
//		}
//		return result.toArray(new Location[result.size()]);
//	}

	/** Creates instance according to name */
	static Location valueOf(const QString & name);

	signed char getRotationOf(Location const & loc) const;

	/**
	 * Check if this rotated another location
	 */
	bool isRotationOf(Location const & loc) const;

	//assertion methods

	bool isFarmLocation() const;

	bool isEdgeLocation() const;

	bool isSpecialLocation() const;

	bool isNull() const
	{
		return mask == 0;
	}
};

inline int qHash(jcz::Location const & l) {
	return l.mask;
}

}

#endif // LOCATION_H
