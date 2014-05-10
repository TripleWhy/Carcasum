#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "static.h"
#include "core/game.h"
#include "core/player.h"
#include "boardgraphicsscene.h"
#include "playerselector.h"
#include "guiIncludes.h"
#include <QComboBox>
#include <QLineEdit>
#include <array>

namespace Ui {
class MainWindow;
}

class PlayerInfoView;

class MainWindow : public QMainWindow, public Player, public ScoreListener, public NextTileProvider
{
	Q_OBJECT

private:
	struct NgPlayerEdit
	{
		QComboBox * colorBox;
		QComboBox * typeBox;
		QLineEdit * nameEdit;
	};

	class GameThread : public QThread
	{
	private:
		Game * g;

	public:
		GameThread(Game * g, QObject * parent = 0) : QThread(parent), g(g) {}
	protected:
		virtual void run();
	};

private:
	Game * game;
	QTimer * timer;
	BoardGraphicsScene * boardUi;
	std::vector<PlayerInfoView *> playerInfos;
	bool isSetUp = false;
	GameThread * gameThread;
	
	jcz::TileFactory tileFactory;
	TileImageFactory imgFactory = TileImageFactory(&tileFactory);
	RandomNextTileProvider rntp;
	bool randomTiles = true;

	const std::array<Qt::GlobalColor, MAX_PLAYERS> colors = {{Qt::red, Qt::blue, Qt::yellow, Qt::darkGreen, Qt::black, Qt::gray}};	//Double brackets not needed in .cpp ...
	const std::array<char const *, MAX_PLAYERS> colorNames= {{   "Red",   "Blue",   "Yellow",       "Green",   "Black",   "Gray"}};
	std::array<NgPlayerEdit, MAX_PLAYERS> ngPlayerEdits;
	PlayerSelector * playerSelector;
	std::array<Player *, MAX_PLAYERS> selectedPlayers = {{}};
	qint64 gameStartTimestamp = -1;

public:
	explicit MainWindow(QWidget *parent = 0);
	~MainWindow();
	
	virtual void newGame(int player, const Game * game);
	virtual void playerMoved(int player, Tile const * tile, MoveHistoryEntry const & move);
	virtual TileMove getTileMove(int player, Tile const * tile, MoveHistoryEntry const & move, TileMovesType const & placements);
	virtual MeepleMove getMeepleMove(int player, Tile const * tile, MoveHistoryEntry const & move, MeepleMovesType const & possible);
	virtual void endGame();
	virtual QString getTypeName() { return "MainWindow"; }
	virtual Player * clone() const;
	virtual void nodeScored(Node const * n, const int score, Game const * game);
	virtual void nodeUnscored(Node const * n, const int score, Game const * game);
	virtual int nextTile(Game const * game);

protected:
	virtual void closeEvent(QCloseEvent *event);

private:
	void readSettings();
	void requestEndGame();
	void forceEndGame();

signals:
	void gameEvent(QString const & msg);
	void updateNeeded();
	void tileDrawn(int player, int tileType);

private slots:
	void displayGameEvent(QString const & msg);
	void timeout();
	void recenter(QRectF rect);
	void colorBoxChanged(int index);
	void typeBoxChanged(int index);

	void on_actionRandom_Tiles_toggled(bool checked);
	void on_actionChoose_Tiles_toggled(bool arg1);
	void on_buttonBox_accepted();
	void on_actionNew_Game_triggered();

	void on_actionStore_board_triggered();

	void on_boardFileButton_clicked();

private:
	Ui::MainWindow *ui;
};

#endif // MAINWINDOW_H
