#include "randomplayer.h"

Move RandomPlayer::getMove(const Tile * const /*tile*/, QList<Board::TilePlacement> const & placements, const Game * const /*game*/)
{
	int i = r.nextInt(placements.size());

	qDebug() << "random" << i << "/" << placements.size();
	Board::TilePlacement const & placement = placements.at(i);
	return Move{placement.x, placement.y, placement.orientation};
}
