#ifndef SIMPLEPLAYER3_H
#define SIMPLEPLAYER3_H

#include "core/player.h"
#include "core/game.h"
#include "core/board.h"
#include "core/random.h"

#define SIMPLE_PLAYER3_RULE_CLOISTER_1    1
#define SIMPLE_PLAYER3_RULE_ROAD_CITY     1
#define SIMPLE_PLAYER3_RULE_FIELD         1
#define SIMPLE_PLAYER3_USE_MEEPLE_PENALTY 1 // This flag only really matters if fields are enabled.
#define SIMPLE_PLAYER3_USE_OPEN_PENALTY   0
#define SIMPLE_PLAYER3_TILE_RANDOM        0 // probability in percent, not a flag
#define SIMPLE_PLAYER3_MEEPLE_RANDOM      0 // probability in percent, not a flag
#define SIMPLE_PLAYER3_NEGATIVE_SCORE_HANDLE_VARIANT 1
#define SIMPLE_PLAYER3_NEGATIVE_SCORE_HANDLE_VARIANT_0_FALLBACK 0

class SimplePlayer3 : public Player
{
private:
	Game const * game;
	Board const * board;
	RandomTable r;
	bool meepleMoveSet;
	MeepleMove meepleMove;

public:
	virtual void newGame(int player, Game const * game);
	virtual void playerMoved(int player, Tile const * tile, MoveHistoryEntry const & move);
	virtual void undoneMove(MoveHistoryEntry const & move);
	virtual TileMove getTileMove(int player, Tile const * tile, MoveHistoryEntry const & move, TileMovesType const & placements);
	virtual MeepleMove getMeepleMove(int player, Tile const * tile, MoveHistoryEntry const & move, MeepleMovesType const & possible);
	virtual void endGame();
	virtual QString getTypeName() const;
	virtual Player * clone() const;



	typedef VarLengthArrayWrapper<std::pair<Move, int>, TILE_ARRAY_LENGTH * NODE_ARRAY_LENGTH>::type RatingsEType;
	static RatingsEType rateAllExpanded(int & sum, Game const * game, const int player, const Tile * tile, const TileMovesType & possible);

	struct NestedMeepleRating
	{
		MeepleMove meepleMove;
		int meepleRating;
	};
	typedef VarLengthArrayWrapper<NestedMeepleRating, NODE_ARRAY_LENGTH>::type RatingsNMeepleType;
	struct NestedTileRating
	{
		TileMove tileMove;
		int tileRating;

		int meepleSum;
#if SIMPLE_PLAYER3_NEGATIVE_SCORE_HANDLE_VARIANT == 1
		int meepleMin;
#endif
		RatingsNMeepleType meepleRatings;
	};
	typedef VarLengthArrayWrapper<NestedTileRating, TILE_ARRAY_LENGTH>::type RatingsNType;
	static RatingsNType rateAllNested(int & sum, Game const * game, const int player, const Tile * tile, const TileMovesType & possible);
};

#endif // SIMPLEPLAYER3_H
