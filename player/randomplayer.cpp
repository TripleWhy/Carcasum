#include "randomplayer.h"

Move RandomPlayer::getMove(const Tile * const /*tile*/, QList<Board::TilePlacement> const & placements, const Game * const /*game*/)
{
	Board::TilePlacement const & placement = placements.at(r.nextInt(placements.size()));
	return Move{placement.x, placement.y, placement.orientation};
}
