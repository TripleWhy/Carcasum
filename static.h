#ifndef STATIC_H
#define STATIC_H

#define _STR(X) #X
#define STR(X) _STR(X)
#define APP_REVISION_STR STR(APP_REVISION)

#define APP_NAME "Carcasum"
#define APP_ORGANIZATION "YMSolutions"

// Some numbers
// Game
#define MAX_PLAYERS                   6
#define MEEPLE_COUNT                  7
#define TILE_COUNT_ARRAY_LENGTH      32
// Player
#define TILE_ARRAY_LENGTH           256
#define NODE_ARRAY_LENGTH            16
// GUI
#define BOARD_TILE_SIZE             300
#define BOARD_MEEPLE_SIZE            75
#define BOARDVIEW_INITIAL_TILE_SIZE  50
#define PINFO_ICON_SIZE              40
#define PINFO_MEEPLE_SIZE            10
#define RTILE_TILE_SIZE              40
#define REMAINING_TILES_COLUMNS       4


// Options
// Tile
#define NODE_VARIANT               0
// Game/MonteCarloPlayer
#define USE_RESET                  0
// RandomTable
#define RANDOM_TABLE_SHARED_OFFSET 0


// Debug options
// Game
#define PRINT_STEPS             0
#define WATCH_SCORES            !defined(QT_NO_DEBUG)
#define CHECK_SIM_STATE         1
// Tile
#define PRINT_CONNECTIONS       0
#define DEBUG_IDS               0
// Ranodm
//#define RANDOM_SEED             17
// BoardGraphicsScene
#define DRAW_TILE_POSITION_TEXT 1
// MonteCarloPlayer
#define COUNT_PLAYOUTS          0
// MCTSPlayer
#define MCTS_COUNT_EXPAND_HITS  1


#endif // STATIC_H
