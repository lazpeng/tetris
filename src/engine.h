#pragma once

#include <stdint.h>

#define BOARD_HEIGHT (40)
#define BOARD_WIDTH (10)
#define BOARD_SIZE (BOARD_HEIGHT * BOARD_WIDTH)
#define EVENT_STACK_SIZE (128)
#define LAST_ROTATION (3)
#define W_WIDTH_DEFAULT (1280)
#define W_HEIGHT_DEFAULT (720)
#define FRAMERATE_DEFAULT (60)
#define BOARD_OFFSET (1)
#define BOARD_ROWS (22)
#define BOARD_COLUMNS (12)
#define DARK_AMOUNT (0.25)
#define FONT_NAME (".\\font.ttf")

typedef struct _tetris_shape_info {
	int width, height;
	int* data;
} tetris_shape_info_t;

extern tetris_shape_info_t g_tetris_shape_table[];
extern int g_tetris_colors[];

typedef struct SDL_Window SDL_Window;
typedef struct SDL_Renderer SDL_Renderer;
typedef struct _TTF_Font TTF_Font;

typedef enum _tetris_shape_kind {
	SHAPE_L_REV = 0,
	SHAPE_L,
	SHAPE_I,
	SHAPE_O,
	SHAPE_S,
	SHAPE_T,
	SHAPE_Z,
	SHAPE_END
} tetris_shape_kind_t;

typedef enum _tetris_color {
	COLOR_CYAN = 0,
	COLOR_YELLOW,
	COLOR_RED,
	COLOR_MAGENTA,
	COLOR_NONE,
	COLOR_GREY,
} tetris_color_t;

typedef struct _tetris_piece {
	int color;
	tetris_shape_kind_t shape;
	int x, y;
	int w, h;
	int *draw_data;
} tetris_piece_t;

typedef struct _tetris_board {
	int cells[BOARD_SIZE];
	tetris_piece_t* current_piece;
} tetris_board_t;

typedef enum _tetris_event_kind {
	EVENT_KEYDOWN,
	EVENT_FOCUS_LOST,
	EVENT_FOCUS_REGAIN
} tetris_event_kind_t;

typedef struct _tetris_event {
	int64_t data;
	tetris_event_kind_t kind;
} tetris_event_t;

typedef enum _tetris_state {
	STATE_HALT,
	STATE_RUNNING,
	STATE_PAUSED
} tetris_state_t;

typedef struct _tetris_context {
	int w_height, w_width;
	SDL_Window* window;
	SDL_Renderer* renderer;
	int target_framerate;
	tetris_state_t game_state;
	tetris_event_t event_stack[EVENT_STACK_SIZE];
	int event_stack_top;
	tetris_board_t board;
	double last_frame_duration;
	TTF_Font* font;
} tetris_context_t;

typedef int (*game_loop_fn_t) (tetris_context_t*);

/**
 *****************************
 * Forward declarations
 *****************************
*/
struct SDL_Renderer;

/**
 *****************************
 * Helpers
 *****************************
*/

int random_number(int upper_limit);

int set_draw_color(SDL_Renderer* renderer, int color);

int darken_color(int color, double amount);

int game_draw(tetris_context_t* ctx);

void rotate_piece(tetris_piece_t* piece);

int game_collect_events(tetris_context_t* ctx);

int game_run(tetris_context_t* ctx, game_loop_fn_t game_update);

void context_destroy(tetris_context_t* ctx);

tetris_context_t* context_create(void);