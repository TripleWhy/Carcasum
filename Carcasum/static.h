/*
	This file is part of Carcasum.

	Carcasum is free software: you can redistribute it and/or modify
	it under the terms of the GNU Affero General Public License as published by
	the Free Software Foundation, either version 3 of the License, or
	(at your option) any later version.

	Carcasum is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU Affero General Public License for more details.

	You should have received a copy of the GNU Affero General Public License
	along with Carcasum.  If not, see <http://www.gnu.org/licenses/>.
*/

#ifndef STATIC_H
#define STATIC_H

#define _STR(X) #X
#define STR(X) _STR(X)
#define APP_REVISION_STR STR(APP_REVISION)

#define APP_NAME "Carcasum"
#define APP_ORGANIZATION "YMSolutions"

// Some numbers
#define TIMEOUT                           5000
// Game
#define MAX_PLAYERS                          6
#define MEEPLE_COUNT                         7
#define TILE_COUNT_ARRAY_LENGTH             32
// Player
#define TILE_ARRAY_LENGTH                  256
#define NODE_ARRAY_LENGTH                   16
// GUI
#define BOARD_TILE_SIZE                    300
#define BOARD_MEEPLE_SIZE                   75
#define BOARD_TILE_FRAME_WIDTH              15
#define BOARDVIEW_INITIAL_TILE_SIZE        100
#define PINFO_ICON_SIZE                     40
#define PINFO_MEEPLE_SIZE                   10
#define RTILE_TILE_SIZE                     40
#define REMAINING_TILES_COLUMNS              4
#define PLAYER_GAP_TIMEOUT                 100
// Util
#define LN_TABLE_SIZE                  1000000
// BoardGraphicsView
#define BOARDGRAPHICSVIEW_ZOOM_SPEED         1.002
#define BOARDGRAPHICSVIEW_MAX_ZOOM           2.0
#define BOARDGRAPHICSVIEW_MIN_ZOOM           0.05
// MCTSPlayer
#define MCTS_NODE_PRIORS_PLAYOUTS           20


// Options
// Tile
#define NODE_VARIANT                 2
// Game/MonteCarloPlayer
#define USE_RESET                    0
// RandomTable
#define RANDOM_TABLE_SHARED_OFFSET   0
// Util
#define USE_BOOST_THREAD_TIMER       1


// Debug options
// Game
#define PRINT_STEPS                  0
#define WATCH_SCORES                 !defined(QT_NO_DEBUG)
#define CHECK_SIM_STATE              0
// Tile
#define PRINT_CONNECTIONS            0
#define DEBUG_IDS                    0
// Ranodm
//#define RANDOM_SEED                  17
// BoardGraphicsScene
#define DRAW_TILE_POSITION_TEXT      0
#define DRAW_NODE_ID_TEXT            0
// MonteCarloPlayer
#define COUNT_PLAYOUTS               0
// MCTSPlayer
#define MCTS_COUNT_EXPAND_HITS       0
#define MCTS_PRINT_UTILITIES         0
// Util
#define OFFSET_ARRAY_ENABLE_CHECKS   0
// MainWindow
#define MAINWINDOW_GAME_ON_STARTUP   0
#define MAINWINDOW_ENABLE_UNDO       0	// Experimental, buggy, but only supposed for debugging.
// main.cpp
#define MAIN_USE_TEST_STATES         0
#define MAIN_RENDER_STATES           0
#define MAIN_STORE_STATES            0
// PlayerInfoView
#define PLAYERINFOVIEW_SCALEABLE     MAIN_RENDER_STATES

#define DISPLAY_WHILE_LOADING        0
#define REPLACE_VARLENGTH_ARRAY      0


#define ASSERT_ENABLED   !(defined(QT_NO_DEBUG) && !defined(QT_FORCE_ASSERTS))


#if REPLACE_VARLENGTH_ARRAY
 #include <vector>
 template <typename T, int U>
 struct VarLengthArrayWrapper
 {
  typedef std::vector<T> type;
 };
#else
 #include <QVarLengthArray>
template <typename T, int U>
struct VarLengthArrayWrapper
{
 typedef QVarLengthArray<T, U> type;
};
#endif

#if DRAW_NODE_ID_TEXT
#undef DEBUG_IDS
#define DEBUG_IDS 1
#endif

#endif // STATIC_H
