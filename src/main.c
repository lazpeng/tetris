#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <time.h>

#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>

#define BOARD_HEIGHT (40)
#define BOARD_WIDTH (10)
#define BOARD_SIZE (BOARD_HEIGHT * BOARD_WIDTH)
#define EVENT_STACK_SIZE (128)
#define LAST_ROTATION (3)
#define W_WIDTH_DEFAULT (1280)
#define W_HEIGHT_DEFAULT (720)
#define FRAMERATE_DEFAULT (60)
#define BOARD_ROWS (10)
#define BOARD_COLUMNS (10)

/**
 *****************************
 * Type definitions
 *****************************
*/

typedef enum _tetris_piece_shape {
	SHAPE_L_REV = 0,
	SHAPE_L,
	SHAPE_I,
	SHAPE_O,
	SHAPE_S,
	SHAPE_T,
	SHAPE_Z,
	SHAPE_END
} tetris_piece_shape_t;

static int g_tetris_shape_table[SHAPE_END][8] =
{
	// *
	// ***
	{ 1, 0, 0, 0, 1, 1, 1, 0 },
	//   *
	// ***
	{ 0, 0, 1, 0, 1, 1, 1, 0 },
	// ****
	{ 1, 1, 1, 1, 0, 0, 0, 0},
	// **
	// **
	{ 1, 1, 0, 0, 1, 1, 0, 0},
	//  **
	// **
	{ 0, 1, 1, 0, 1, 1, 0, 0 },
	//  *
	// ***
	{ 0, 1, 0, 0, 1, 1, 1, 0 },
	// **
	//  **
	{ 1, 1, 0, 0, 0, 1, 1, 0 }
};

typedef enum _tetris_color {
	COLOR_CYAN = 0,
	COLOR_YELLOW,
	COLOR_RED,
	COLOR_MAGENTA,
	COLOR_END
} tetris_color_t;

typedef enum _tetris_direction {
	DIRECTION_HORIZONTAL,
	DIRECTION_VERTICAL
} tetris_direction_t;

int g_tetris_colors[] = { 0x21d5dbff, 0xe8e225ff, 0xd10804ff, 0xce04d1ff };

typedef struct _tetris_piece {
	int color;
	tetris_piece_shape_t shape;
	int x, y;
	int rotation_index;
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
} tetris_context_t;

typedef int (*game_loop_fn_t) (tetris_context_t*);

/**
 *****************************
 * Helpers
 *****************************
*/
static int random_number(int upper_limit) {
	srand(time(NULL));

	return rand() % upper_limit;
}

static int set_draw_color(SDL_Renderer* renderer, int color) {
	int r, g, b, a;

	a = color & 0xFF;
	b = (color >> 8) & 0xFF;
	g = (color >> 16) & 0xFF;
	r = (color >> 24) & 0xFF;

	SDL_SetRenderDrawColor(renderer, r, g, b, a);
}

/**
 *****************************
 * Initialization
 *****************************
*/

void board_initialize(tetris_board_t* board) {
	memset(board->cells, 0, sizeof(board));
	board->current_piece = NULL;
}

void board_destroy(tetris_board_t* board) {
	if (board->current_piece != NULL) {
		free(board->current_piece);
	}
}

void board_spawn_piece(tetris_board_t* board) {
	if (board->current_piece != NULL) {
		free(board->current_piece);
		board->current_piece = NULL;
	}

	tetris_piece_t* piece = calloc(1, sizeof(*piece));

	piece->color = g_tetris_colors[random_number(COLOR_END)];
	piece->shape = random_number(SHAPE_END);
	piece->x = 5;
	piece->y = 5;
	piece->rotation_index = 0;

	board->current_piece = piece;
}

tetris_context_t* context_create(void) {
	tetris_context_t* ctx = calloc(1, sizeof(*ctx));

	ctx->w_width = W_WIDTH_DEFAULT;
	ctx->w_height = W_HEIGHT_DEFAULT;

	int position = SDL_WINDOWPOS_CENTERED;

	ctx->window = SDL_CreateWindow("Not tetris", position, position, ctx->w_width, ctx->w_height, SDL_WINDOW_SHOWN);

	if (ctx->window == NULL) {
		puts("Failed to create window");
		free(ctx);
		return NULL;
	}

	ctx->renderer = SDL_CreateRenderer(ctx->window, -1, SDL_RENDERER_ACCELERATED);

	if (ctx->renderer == NULL) {
		puts("Failed to create accelerated renderer. Trying software.");

		ctx->renderer = SDL_CreateRenderer(ctx->window, -1, SDL_RENDERER_SOFTWARE);
		if (ctx->renderer == NULL) {
			SDL_DestroyWindow(ctx->window);
			puts("Failed to create software renderer.");
			free(ctx);
			return NULL;
		}
	}

	ctx->target_framerate = FRAMERATE_DEFAULT;
	ctx->event_stack_top = 0;
	ctx->game_state = STATE_HALT;
	ctx->last_frame_duration = 0;

	board_initialize(&ctx->board);

	return ctx;
}

/**
 *****************************
 * Event handling
 *****************************
*/

int game_push_event(tetris_context_t* ctx, tetris_event_kind_t kind, int64_t data) {
	if (ctx->event_stack_top >= EVENT_STACK_SIZE) {
		puts("Fatal error: Event stack overflow");
		return 2;
	}

	tetris_event_t* event = &(ctx->event_stack[ctx->event_stack_top++]);
	event->data = data;
	event->kind = kind;

	return 0;
}

int game_collect_events(tetris_context_t* ctx) {
	SDL_Event e;

	while (SDL_PollEvent(&e)) {
		switch (e.type) {
		case SDL_KEYDOWN:
			return game_push_event(ctx, EVENT_KEYDOWN, e.key.keysym.sym);
			break;
		case SDL_WINDOWEVENT_FOCUS_GAINED:
		case SDL_WINDOWEVENT_RESTORED:
			return game_push_event(ctx, EVENT_FOCUS_REGAIN, 0);
			break;
		case SDL_WINDOWEVENT_FOCUS_LOST:
		case SDL_WINDOWEVENT_MINIMIZED:
			return game_push_event(ctx, EVENT_FOCUS_LOST, 0);
			break;
		case SDL_WINDOWEVENT_MOVED:
			SDL_GetWindowPosition(ctx->window, e.window.data1, e.window.data2);
			break;
		case SDL_WINDOWEVENT_CLOSE:
		case SDL_QUIT:
			puts("Game closing...");
			return 1;
		default:
			return 0;
		}
	}
}

/**
 *****************************
 * Game update logic
 *****************************
*/

int game_update(tetris_context_t* ctx) {
	int i;
	for (int i = 0; i < ctx->event_stack_top; ++i) {
		tetris_event_t* ev = &ctx->event_stack[i];
		if (ev->kind == EVENT_KEYDOWN) {
			tetris_piece_t* piece = ctx->board.current_piece;
			if (piece != NULL) {
				switch (ev->data) {
				case SDLK_LEFT:
				case SDLK_a:
					if (piece->x > 0) {
						piece->x -= 1;
					}
					break;
				case SDLK_RIGHT:
				case SDLK_d:
					if (piece->x < 10) {
						piece->x += 1;
					}
					break;
				case SDLK_UP:
				case SDLK_w:
					if (piece->y > 0) {
						piece->y -= 1;
					}
					break;
				case SDLK_DOWN:
				case SDLK_s:
					if (piece->y < 20) {
						piece->y += 1;
					}
					break;
				case SDLK_r:
					if (piece->rotation_index == 3) {
						piece->rotation_index = 0;
					}
					else {
						piece->rotation_index += 1;
					}
					break;
				default:
					break;
				}
			}
		}
	}
	ctx->event_stack_top = 0;

	return 0;
}

/**
 *****************************
 * Game drawing code
 *****************************
*/

void query_board_size(tetris_context_t* ctx, int* width, int* height) {
	if (height != NULL) {
		*height = ctx->w_height;
	}

	if (width != NULL) {
		*width = ctx->w_width * 0.60;
	}
}

int draw_existing_blocks(tetris_context_t* ctx) {
	return 0;
}

int is_row_all_zeroes(const int* row, int count) {
	int i;
	for (i = 0; i < count; ++i) {
		if (row[i] != 0) {
			return 0;
		}
	}

	return 1;
}

void draw_cell(tetris_context_t* ctx, SDL_Point base_coord, int offset, tetris_direction_t direction, int cell) {
	if (cell == 0) {
		return;
	}

	int bw, bh;

	query_board_size(ctx, &bw, &bh);

	const int cell_width = bw / BOARD_COLUMNS;
	const int cell_height = bh / BOARD_ROWS;

	SDL_Rect rect;

	rect.h = cell_height;
	rect.w = cell_width;

	rect.x = base_coord.x * cell_width;
	rect.y = base_coord.y * cell_height;

	if (direction == DIRECTION_HORIZONTAL) {
		rect.x += offset * cell_width;
	}
	else {
		rect.y += offset * cell_height;
	}

	SDL_RenderFillRect(ctx->renderer, &rect);
}

void draw_piece_row(tetris_context_t *ctx, tetris_piece_t *piece, int offset, SDL_Point coord, tetris_direction_t direction, int reverse) {
	const int* data = g_tetris_shape_table[piece->shape];

	set_draw_color(ctx->renderer, piece->color);

	int pixel_offset = 0;
	int i;

	if (reverse) {
		for (i = 3; i >= 0; --i) {
			draw_cell(ctx, coord, pixel_offset++, direction, data[offset + i]);
		}
	}
	else {
		for (i = 0; i < 4; ++i) {
			draw_cell(ctx, coord, pixel_offset++, direction, data[offset + i]);
		}
	}
}

int draw_current_piece(tetris_context_t* ctx) {
	const tetris_piece_t* piece = ctx->board.current_piece;

	if (piece == NULL) {
		return 0;
	}

	const int* shape_data = g_tetris_shape_table[ctx->board.current_piece->shape];

	tetris_direction_t direction = piece->rotation_index % 2 == 0 ? DIRECTION_HORIZONTAL : DIRECTION_VERTICAL;

	int col_reverse = piece->rotation_index != 0 && piece->rotation_index != 3;
	int row_reverse = piece->rotation_index > 1;

	SDL_Point coord;
	coord.x = piece->x;
	coord.y = piece->y;

	draw_piece_row(ctx, piece, col_reverse ? 4 : 0, coord, direction, row_reverse);

	if (direction == DIRECTION_HORIZONTAL) {
		coord.y += 1;
	}
	else {
		coord.x += 1;
	}

	draw_piece_row(ctx, piece, col_reverse ? 0 : 4, coord, direction, row_reverse);

	return 0;
}

int draw_score(tetris_context_t* ctx) {

	return 0;
}

int game_draw(tetris_context_t* ctx) {
	int status_code = 0;

	game_loop_fn_t draw_functions[] = { draw_existing_blocks, draw_current_piece, draw_score };

	SDL_SetRenderDrawColor(ctx->renderer, 0, 0, 0, 255);
	SDL_RenderClear(ctx->renderer);

	int i;
	for (i = 0; i < sizeof draw_functions / sizeof * draw_functions; ++i) {
		game_loop_fn_t fn = draw_functions[i];

		status_code = fn(ctx);
		if (status_code != 0) {
			puts("Error occurred during drawing");
			break;
		}
	}

	SDL_RenderPresent(ctx->renderer);

	return status_code;
}

/**
 *****************************
 * Actual game loop
 *****************************
 */

void game_update_title(tetris_context_t* ctx) {
	static char buffer[1024];
	*buffer = 0;

	double framerate = 1000.0 / (ctx->last_frame_duration * 1000);

	snprintf(buffer, sizeof(buffer), "Not Tetris - %dx%d - %.2f FPS", ctx->w_width, ctx->w_height, framerate);
	SDL_SetWindowTitle(ctx->window, buffer);
}

int game_run(tetris_context_t* ctx) {
	int status_code;

	const int target_ticks = (1000 / ctx->target_framerate);

	game_loop_fn_t game_loop_functions[] = { game_collect_events, game_update, game_draw };

	board_spawn_piece(&ctx->board);

	do {
		uint64_t frame_ticks = SDL_GetTicks();

		int i;
		for (i = 0; i < sizeof game_loop_functions / sizeof(*game_loop_functions); ++i) {
			game_loop_fn_t fun = game_loop_functions[i];
			status_code = fun(ctx);
			if (status_code != 0) {
				break;
			}
		}

		uint64_t elapsed = SDL_GetTicks() - frame_ticks;
		if (elapsed < target_ticks) {
			SDL_Delay(target_ticks - elapsed);
		}

		elapsed = SDL_GetTicks() - frame_ticks;

		ctx->last_frame_duration = elapsed / 1000.0;

		game_update_title(ctx);
	} while (1);

	return status_code;
}

void context_destroy(tetris_context_t* ctx) {
	SDL_DestroyRenderer(ctx->renderer);
	SDL_DestroyWindow(ctx->window);

	board_destroy(&ctx->board);

	free(ctx);
}

int main(int argc, char** argv) {
	puts("Initializing SDL2...");

	if (SDL_Init(SDL_INIT_EVERYTHING) != 0) {
		puts("Failed to initialize SDL");
		return EXIT_FAILURE;
	}

	puts("Initializing context...");

	tetris_context_t* ctx = context_create();

	puts("Game start");

	int status_code = game_run(ctx);

	context_destroy(ctx);

	return status_code;
}