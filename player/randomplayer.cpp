#include "randomplayer.h"

TileMove RandomPlayer::getTileMove(int /*player*/, const Tile * const /*tile*/, QList<TileMove> const & placements, const Game * const /*game*/)
{
	int i = r.nextInt(placements.size());

	qDebug() << "random" << i << "/" << placements.size();
	TileMove const & placement = placements.at(i);
	//return Move{placement.x, placement.y, placement.orientation};
	return TileMove(placement.x, placement.y, placement.orientation);
}

MeepleMove RandomPlayer::getMeepleMove(int /*player*/, const Tile * const /*tile*/, const QVarLengthArray<MeepleMove, NODE_ARRAY_LENGTH> & possible, const Game * const /*game*/)
{
	int i = r.nextInt(possible.size());

	qDebug() << "random" << i << "/" << possible.size();
	return possible.at(i);
}
