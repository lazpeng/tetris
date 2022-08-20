#pragma once

#include <stdint.h>
#include <stdbool.h>

#define EVENT_STACK_SIZE (128)
#define W_WIDTH_DEFAULT (900)
#define W_HEIGHT_DEFAULT (600)
#define FRAMERATE_DEFAULT (60)
#define BOARD_ROWS (22)
#define BOARD_COLUMNS (12)
#define BOARD_SIZE (BOARD_ROWS * BOARD_COLUMNS)
#define DARK_AMOUNT (0.25)

#ifdef __APPLE__
# define FONT_LOCATION "/System/Library/Fonts/"
#define FONT_NAME "Monaco.ttf"
#elif defined(WIN32) || defined(_WIN32)
# define FONT_LOCATION "C:\\Windows\\Fonts\\"
#define FONT_NAME "Consolas.ttf"
#else
# define FONT_LOCATION "/usr/share/fonts/"
#define FONT_NAME "Consolas.ttf"
#endif

#define SCORE_BASE_SINGLE 10	/* Base score for clearing a single row.		*/
#define SCORE_BASE_DOUBLE 20	/* Base score for clearing two rows.			*/
#define SCORE_BASE_TRIPLE 40	/* Base score for clearing three rows.			*/
#define SCORE_BASE_TETRIS 80	/* Base score for clearing four our more rows.	*/

#define FALL_TIME_SECONDS_BEG 0.50	/* Fall time for a piece at early game.	*/
#define FALL_TIME_SECONDS_END 0.20	/* Fall time for a piece at late game.	*/
/* From no score to this ammount of score, the fall time will be interpolated
 * between the beggining and ending values. For scores larger than this number,
 * the fall time for a piece will be fixed at the ending value. */
#define FALL_TIME_SCORE_RANGE 2000

typedef struct {
    int width, height;
    int *data;
} tetris_shape_info_t;

extern tetris_shape_info_t g_tetris_shape_table[];
extern int g_tetris_colors[];

typedef struct SDL_Window SDL_Window;
typedef struct SDL_Renderer SDL_Renderer;
typedef struct SDL_Texture SDL_Texture;
typedef struct _TTF_Font TTF_Font;

typedef enum {
    SHAPE_L_REV = 0,
    SHAPE_L,
    SHAPE_I,
    SHAPE_O,
    SHAPE_S,
    SHAPE_T,
    SHAPE_Z,
    SHAPE_END
} tetris_shape_kind_t;

typedef enum {
    COLOR_CYAN = 0,
    COLOR_YELLOW,
    COLOR_RED,
    COLOR_MAGENTA,
    COLOR_NONE,
    COLOR_MARGIN,
} tetris_color_t;

typedef struct {
    int color;
    tetris_shape_kind_t shape;
    int x, y;
    int w, h;
    int *draw_data;
} tetris_piece_t;

typedef struct {
    int cells[BOARD_SIZE];
    tetris_piece_t *current_piece;
} tetris_board_t;

typedef enum {
    EVENT_KEYDOWN,
    EVENT_FOCUS_LOST,
    EVENT_FOCUS_REGAIN
} tetris_event_kind_t;

typedef struct {
    int64_t data;
    tetris_event_kind_t kind;
} tetris_event_t;

typedef struct {
    int pieces_spawned, lines_cleared;
    uint64_t start_time, end_time;
} tetris_stats_t;

typedef struct {
	int w_height, w_width;
	SDL_Window* window;
	SDL_Renderer* renderer;
    SDL_Texture* score_texture;
	int target_framerate;
	tetris_event_t event_stack[EVENT_STACK_SIZE];
	int event_stack_top;
	tetris_board_t board;
	double last_frame_duration;
	double last_delta_time;
	uint64_t last_time;
	double fall_timer;
	unsigned int score;
	tetris_stats_t stats;
	TTF_Font* font;
    bool paused;
} tetris_context_t;

typedef int (*game_loop_fn_t)(tetris_context_t *);

/**
 *****************************
 * Forward declarations
 *****************************
*/
struct SDL_Renderer;

int random_number(int upper_limit);

int set_draw_color(SDL_Renderer *renderer, uint32_t color);

int darken_color(uint32_t color, double amount);

int game_draw(tetris_context_t *ctx);

double game_get_piece_fall_time(tetris_context_t* ctx);

int board_get_cell(const tetris_board_t *board, int x, int y);

void board_spawn_piece(tetris_context_t *ctx);

void board_fixate_current_piece(tetris_board_t *board);

void board_check_for_clears(tetris_context_t *ctx);

int game_collect_events(tetris_context_t *ctx);

int game_run(tetris_context_t *ctx, game_loop_fn_t game_update);

void context_destroy(tetris_context_t *ctx);

tetris_context_t *context_create(void);

void context_reset(tetris_context_t *ctx);
