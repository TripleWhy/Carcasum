#ifndef METAPLAYERS_H
#define METAPLAYERS_H

#include "core/player.h"
#include "core/game.h"
#include "core/random.h"
#include "player/simpleplayer3.h"

template<int RndPercent, typename Pl = SimplePlayer3>
class EGreedyPlayer : public Player
{
private:
	Pl m_player;
	RandomTable r;
	const QString name = QString("EGreedyPlayer<%1, %2>").arg(RndPercent).arg(m_player.getTypeName());

public:
	constexpr EGreedyPlayer() {}
	constexpr EGreedyPlayer(EGreedyPlayer &&) = default;

	virtual void newGame(int player, Game const * game);
	virtual void playerMoved(int player, Tile const * tile, MoveHistoryEntry const & move);
	virtual void undoneMove(MoveHistoryEntry const & move);
	virtual TileMove getTileMove(int player, Tile const * tile, MoveHistoryEntry const & move, TileMovesType const & possible);
	virtual MeepleMove getMeepleMove(int player, Tile const * tile, MoveHistoryEntry const & move, MeepleMovesType const & possible);
	virtual void endGame();
	virtual QString getTypeName() const;
	virtual Player * clone() const;
};

class RouletteWheelPlayer : public Player
{
private:
	RandomTable r;
	Game const * game;
	MeepleMove meepleMove;

public:
	virtual void newGame(int player, Game const * game);
	virtual void playerMoved(int player, Tile const * tile, MoveHistoryEntry const & move);
	virtual void undoneMove(MoveHistoryEntry const & move);
	virtual TileMove getTileMove(int player, Tile const * tile, MoveHistoryEntry const & move, TileMovesType const & possible);
	virtual MeepleMove getMeepleMove(int player, Tile const * tile, MoveHistoryEntry const & move, MeepleMovesType const & possible);
	virtual void endGame();
	virtual QString getTypeName() const;
	virtual Player * clone() const;
};

class RouletteWheelPlayer2 : public Player
{
private:
	RandomTable r;
	Game const * game;
	SimplePlayer3::RatingsNMeepleType meepleRatings;
	int meepleSum;

public:
	virtual void newGame(int player, Game const * game);
	virtual void playerMoved(int player, Tile const * tile, MoveHistoryEntry const & move);
	virtual void undoneMove(MoveHistoryEntry const & move);
	virtual TileMove getTileMove(int player, Tile const * tile, MoveHistoryEntry const & move, TileMovesType const & possible);
	virtual MeepleMove getMeepleMove(int player, Tile const * tile, MoveHistoryEntry const & move, MeepleMovesType const & possible);
	virtual void endGame();
	virtual QString getTypeName() const;
	virtual Player * clone() const;
};

#endif // METAPLAYERS_H
