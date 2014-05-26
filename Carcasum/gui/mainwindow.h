#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "static.h"
#include "core/util.h"
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

	class GameThread : public Util::InterruptableThread
	{
	private:
		Game * g;
		std::atomic<int> undoSteps;

	public:
		GameThread(Game * g, QObject * parent = 0) : Util::InterruptableThread(parent), g(g), undoSteps(0) {}
		void undo(int steps = 1);
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

	const std::array<Qt::GlobalColor, MAX_PLAYERS> colors = {{Qt::red,   Qt::blue,   Qt::yellow, Qt::darkGreen,   Qt::black,   Qt::gray}};	//Double brackets not needed in .cpp ...
	const std::array<QString, MAX_PLAYERS> colorNames     = {{tr("Red"), tr("Blue"), tr("Yellow"),   tr("Green"), tr("Black"), tr("Gray")}};
	std::array<NgPlayerEdit, MAX_PLAYERS> ngPlayerEdits;
	PlayerSelector * playerSelector;
	std::array<Player *, MAX_PLAYERS> selectedPlayers = {{}};
	qint64 gameStartTimestamp = -1;

public:
	explicit MainWindow(QWidget *parent = 0);
	~MainWindow();
	
	virtual void newGame(int player, const Game * game);
	virtual void playerMoved(int player, Tile const * tile, MoveHistoryEntry const & move);
	virtual void undoneMove(MoveHistoryEntry const & move);
	virtual TileMove getTileMove(int player, Tile const * tile, MoveHistoryEntry const & move, TileMovesType const & placements);
	virtual MeepleMove getMeepleMove(int player, Tile const * tile, MoveHistoryEntry const & move, MeepleMovesType const & possible);
	virtual void endGame();
	virtual QString getTypeName() const { return "MainWindow"; }
	virtual Player * clone() const;
	virtual void nodeScored(Node const * n, const int score, Game const * game);
	virtual void nodeUnscored(Node const * n, const int score, Game const * game);
	virtual int nextTile(Game const * game);

protected:
	virtual void closeEvent(QCloseEvent *event);
#if MAINWINDOW_GAME_ON_STARTUP
	virtual bool event(QEvent * event);
#endif

private:
	void readSettings();
	void requestEndGame();
	void forceEndGame();

signals:
	void gameEvent(QString const & msg);
	void gameEventPop();
	void updateNeeded();
	void tileDrawn(int player, int tileType);

private slots:
	void displayGameEvent(QString const & msg);
	void displayGameEventPop();
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
	void on_actionControls_triggered();
	void on_actionUndo_triggered();

private:
	Ui::MainWindow *ui;
};

#endif // MAINWINDOW_H
