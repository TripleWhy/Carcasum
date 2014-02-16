#include "game.h"

Game::Game()
	: ply(-1),
	  board(0)
{
}

Game::~Game()
{
	cleanUp();
}

void Game::newGame(Tile::TileSets tileSets)
{
	if (players.size() == 0)
		return;

	cleanUp();

	ply = 0;
	tiles = TileFactory::createTiles(tileSets);
	board = new Board(tiles.size());
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
	QList<Board::TilePlacement> && placements = board->getPossibleTilePlacements(tile);
	while (placements.isEmpty())
	{
		tile = tiles.takeAt(r.nextInt(tiles.size()));
		placements = board->getPossibleTilePlacements(tile);
	}
	qDebug() << "possible tile placements:" << placements.size();

	int playerIndex = ply % players.size();
	Player * player = players[playerIndex];
	Move move = player->getMove(tile, placements, this);

	tile->orientation = move.orientation;
	//TODO meeple

	board->addTile(move.x, move.y, tile);
	emit boardChanged(board);
}

void Game::cleanUp()
{
	delete board;
	qDeleteAll(tiles);
	tiles.clear();
}
