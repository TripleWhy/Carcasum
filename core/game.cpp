#include "game.h"

#include "player.h"
#include "jcz/jczutils.h"

Game::Game()
	: ply(-1),
	  board(0)
{
}

Game::~Game()
{
	cleanUp();
}

void Game::newGame(Tile::TileSets tileSets, JCZUtils::TileFactory * tileFactory)
{
	if (players.size() == 0)
		return;

	cleanUp();

	ply = 0;
	tiles = tileFactory->createPack(tileSets);
	board = new Board(this, tiles.size());
	board->setStartTile(tiles.takeFirst());

	emit boardChanged(board);
}

void Game::addPlayer(Player * player)
{
	if (!isFinished())
		return;
	players.append(player);
}

void Game::setPlayer(int index, Player * player)
{
	players[index] = player;
	//TODO player->setGameState oder so
}

void Game::setPlayers(QList<Player *> players)
{
	if (!isFinished())
		return;
	this->players = players;
}

void Game::clearPlayers()
{
	if (!isFinished())
		return;
	players.clear();
}

Board const * Game::getBoard() const
{
	return board;
}

bool Game::isFinished()
{
	return ply < 0;
}

void Game::step()
{
	if (isFinished())
		return;

	Tile * tile = tiles.takeAt(r.nextInt(tiles.size()));
//	Tile * tile = tiles.takeAt(0);
	QList<Board::TilePlacement> && placements = board->getPossibleTilePlacements(tile);
	while (placements.isEmpty())
	{
		delete tile;
		tile = tiles.takeAt(r.nextInt(tiles.size()));
		placements = board->getPossibleTilePlacements(tile);
	}
	qDebug() << "tile type:" << tile->tileType << char('A' + tile->tileType) << "possible placements:" << placements.size();

	int playerIndex = ply % players.size();
	Player * player = players[playerIndex];
	Move move = player->getMove(tile, placements, this);
//	Move move{72, 73, Tile::left};
	qDebug() << "player" << playerIndex << move.x << move.y << move.orientation;

	tile->orientation = move.orientation;
	//TODO meeple

	board->addTile(move.x, move.y, tile);
	emit boardChanged(board);

	if (tiles.isEmpty())
		ply = -1;
	else
		++ply;
}

void Game::cityClosed(CityNode * n)
{
	int score = n->tiles.size();
	if (score > 2)
		score = (score + n->bonus) * 2;
	qDebug() << "   city closed, value:" << score;
}

void Game::roadClosed(RoadNode * n)
{
	int score = n->tiles.size();
	qDebug() << "   raod closed, value:" << score;
}

void Game::cleanUp()
{
	delete board;
	qDeleteAll(tiles);
	tiles.clear();
}
