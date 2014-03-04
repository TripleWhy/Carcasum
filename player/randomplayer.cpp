#include "randomplayer.h"

TileMove RandomPlayer::getTileMove(const Tile * const /*tile*/, QList<Board::TilePlacement> const & placements, const Game * const /*game*/)
{
	int i = r.nextInt(placements.size());

	qDebug() << "random" << i << "/" << placements.size();
	Board::TilePlacement const & placement = placements.at(i);
	//return Move{placement.x, placement.y, placement.orientation};
	return TileMove(placement.x, placement.y, placement.orientation);
}

MeepleMove RandomPlayer::getMeepleMove(const Tile * const /*tile*/, const QVarLengthArray<MeepleMove, NODE_ARRAY_LENGTH> & possible, const Game * const /*game*/)
{
	int i = r.nextInt(possible.size());

	qDebug() << "random" << i << "/" << possible.size();
	return possible.at(i);
}
