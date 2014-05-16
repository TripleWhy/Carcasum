#include "jczplayer.h"
#ifdef JCZPLAYER_H

JCZPlayer::JCZPlayer()
{
}

JCZPlayer::~JCZPlayer()
{

}

void JCZPlayer::newGame(int player, const Game * game)
{

}

void JCZPlayer::playerMoved(int player, const Tile * tile, const MoveHistoryEntry & move)
{

}

TileMove JCZPlayer::getTileMove(int player, const Tile * tile, const MoveHistoryEntry & move, const TileMovesType & placements)
{
	return TileMove();
}

MeepleMove JCZPlayer::getMeepleMove(int player, const Tile * tile, const MoveHistoryEntry & move, const MeepleMovesType & possible)
{
	return MeepleMove();
}

void JCZPlayer::endGame()
{

}

QString JCZPlayer::getTypeName()
{
	return "JCZPlayer";
}

Player *JCZPlayer::clone() const
{
	return 0;
}
#endif
