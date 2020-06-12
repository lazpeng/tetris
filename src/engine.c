#include "engine.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <stdint.h>

#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>

int g_tetris_colors[] = { 0x21d5db, 0xe8e225, 0xd10804, 0xce04d1, 0x333333, 0x777777 };

// *
// ***
static int g_shape_rev_l[] = { 1, 0, 0, 1, 1, 1 };

//   *
// ***
static int g_shape_l[] = { 0, 0, 1, 1, 1, 1 };

// ****
//
static int g_shape_i[] = { 1, 1, 1, 1 };

// **
// **
static int g_shape_o[] = { 1, 1, 1, 1 };

//  **
// **
static int g_shape_s[] = { 0, 1, 1, 1, 1, 0 };

//  *
// ***
static int g_shape_t[] = { 0, 1, 0, 1, 1, 1 };

// **
//  **
static int g_shape_z[] = { 1, 1, 0, 0, 1, 1 };

tetris_shape_info_t g_tetris_shape_table[] =
{
	{ .width = 3, .height = 2, .data = g_shape_rev_l },
	{ .width = 3, .height = 2, .data = g_shape_l },
	{ .width = 4, .height = 1, .data = g_shape_i },
	{ .width = 2, .height = 2, .data = g_shape_o },
	{ .width = 3, .height = 2, .data = g_shape_s },
	{ .width = 3, .height = 2, .data = g_shape_t },
	{ .width = 3, .height = 2, .data = g_shape_z }
};

int random_number(int upper_limit) {
	srand(time(NULL));

	return rand() % upper_limit;
}

int set_draw_color(SDL_Renderer* renderer, uint32_t color) {
	int r, g, b;

	b = color & 0xFF;
	g = (color >> 8) & 0xFF;
	r = (color >> 16) & 0xFF;

	SDL_SetRenderDrawColor(renderer, r, g, b, SDL_ALPHA_OPAQUE);
	return 0;
}

int darken_color(uint32_t color, double amount) {
	uint32_t r, g, b;

	b = (color >> 0) & 0xFF;
	g = (color >> 8) & 0xFF;
	r = (color >> 16) & 0xFF;

	r = (double)r * (1 - amount);
	g = (double)g * (1 - amount);
	b = (double)b * (1 - amount);

	return (r << 16) | (g << 8) | b;
}

void board_initialize(tetris_board_t* board) {
	board->current_piece = NULL;

	int grey_bg = g_tetris_colors[COLOR_NONE];
	int grey = g_tetris_colors[COLOR_GREY];

	int i;
	for (i = 0; i < BOARD_ROWS; ++i) {
		int j;
		for (j = 0; j < BOARD_COLUMNS; ++j) {
			if (j != 0 && j != BOARD_COLUMNS - 1 && i != 0 && i != BOARD_ROWS - 1) {
				board->cells[i * BOARD_COLUMNS + j] = grey_bg;
			}
			else {
				board->cells[i * BOARD_COLUMNS + j] = grey;
			}
		}
	}
}

void board_destroy(tetris_board_t* board) {
	if (board->current_piece != NULL) {
		if (board->current_piece->draw_data != NULL) {
			free(board->current_piece->draw_data);
		}
		free(board->current_piece);
	}
}

void board_fixate_current_piece(tetris_board_t *board) {
    if(board->current_piece == NULL) {
        return;
    }

    const tetris_piece_t * piece = board->current_piece;

    int row;
    for(row = 0; row < piece->h; ++row) {
        int col;
        for(col = 0; col < piece->w; ++col) {
            if(piece->draw_data[row * piece->w + col] != 0) {
                const int xpos = piece->x + col;
                const int ypos = piece->y + row;
                board->cells[ypos * BOARD_COLUMNS + xpos] = piece->color;
            }
        }
    }
}

void board_spawn_piece(tetris_board_t* board) {
	if (board->current_piece != NULL) {
		free(board->current_piece);
		board->current_piece = NULL;
	}

	tetris_piece_t* piece = calloc(1, sizeof(*piece));

	piece->color = g_tetris_colors[random_number(COLOR_NONE)];
	piece->shape = random_number(SHAPE_END);
	piece->x = 5;
	piece->y = 1;
	
	tetris_shape_info_t info = g_tetris_shape_table[piece->shape];
	piece->w = info.width;
	piece->h = info.height;

	const int size = piece->w * piece->h * sizeof (int);

	piece->draw_data = calloc(1, size);

	memcpy(piece->draw_data, info.data, size);

	board->current_piece = piece;
}

tetris_context_t* context_create(void) {
	puts("Initializing SDL2...");

	if (SDL_Init(SDL_INIT_EVERYTHING) != 0) {
		puts("Failed to initialize SDL");
		puts(SDL_GetError());
		return NULL;
	}

	puts("Initializing SDL_TTF...");

	if (TTF_Init() != 0) {
		puts("Failed to initialize SDL_TTF");
		puts(TTF_GetError());
		SDL_Quit();
		return NULL;
	}

	puts("Initializing context...");

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

	SDL_SetRenderDrawBlendMode(ctx->renderer, SDL_BLENDMODE_BLEND);

	ctx->target_framerate = FRAMERATE_DEFAULT;
	ctx->event_stack_top = 0;
	ctx->game_state = STATE_HALT;
	ctx->last_frame_duration = 0;
	ctx->font = NULL;

	board_initialize(&ctx->board);

	// FIXME
	board_spawn_piece(&ctx->board);

	return ctx;
}

void context_destroy(tetris_context_t* ctx) {
	SDL_DestroyRenderer(ctx->renderer);
	SDL_DestroyWindow(ctx->window);

	board_destroy(&ctx->board);

	free(ctx);

	TTF_Quit();
	SDL_Quit();
}

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
		case SDL_WINDOWEVENT_FOCUS_GAINED:
		case SDL_WINDOWEVENT_RESTORED:
			return game_push_event(ctx, EVENT_FOCUS_REGAIN, 0);
		case SDL_WINDOWEVENT_FOCUS_LOST:
		case SDL_WINDOWEVENT_MINIMIZED:
			return game_push_event(ctx, EVENT_FOCUS_LOST, 0);
		case SDL_WINDOWEVENT_MOVED:
			SDL_SetWindowPosition(ctx->window, e.window.data1, e.window.data2);
			break;
		case SDL_WINDOWEVENT_CLOSE:
		case SDL_QUIT:
			return 1;
		default:
			return 0;
		}
	}

	return 0;
}

void query_board_size(tetris_context_t* ctx, double* width, double* height) {
	const double vert_region = ctx->w_height;
	const double hori_region = vert_region * ((double)BOARD_COLUMNS / (double)BOARD_ROWS);

	if (height != NULL) {
		*height = vert_region;
	}

	if (width != NULL) {
		*width = hori_region;
	}
}

void draw_single_block(tetris_context_t* ctx, int x, int y, int color) {
	double bw, bh;

	query_board_size(ctx, &bw, &bh);

	const double cell_width = bw / BOARD_COLUMNS;
	const double cell_height = bh / BOARD_ROWS;

	SDL_FRect rect;

	rect.h = cell_height;
	rect.w = cell_width;
	rect.x = x * cell_width;
	rect.y = y * cell_height;

	const int inner_color = darken_color(color, DARK_AMOUNT);

	set_draw_color(ctx->renderer, color);
	SDL_RenderFillRectF(ctx->renderer, &rect);

	set_draw_color(ctx->renderer, inner_color);
	SDL_RenderDrawRectF(ctx->renderer, &rect);
}

int board_get_cell(const tetris_board_t *board, int x, int y) {
    return board->cells[y * BOARD_COLUMNS + x];
}

int draw_existing_blocks(tetris_context_t* ctx) {
	int i, x = 0, y = 0;
	for (i = 0; i < BOARD_SIZE; ++i) {
		if (x % BOARD_COLUMNS == 0 && i > 0) {
			y += 1;
			x = 0;
		}

		draw_single_block(ctx, x, y, ctx->board.cells[i]);

		x += 1;
	}

	return 0;
}

int draw_current_piece(tetris_context_t* ctx) {
	const tetris_piece_t* piece = ctx->board.current_piece;

	if (piece != NULL) {
		int y;
		for (y = 0; y < piece->h; ++y) {
			int x;
			for (x = 0; x < piece->w; ++x) {
				if (piece->draw_data[y * piece->w + x]) {
					draw_single_block(ctx, piece->x + x, piece->y + y, piece->color);
				}
			}
		}
	}

	return 0;
}

void game_rotate_piece(tetris_board_t* board) {
    tetris_piece_t *piece = board->current_piece;

    if(piece == NULL) {
        return;
    }

	int* buffer = calloc(1, sizeof(int) * piece->w * piece->h);

	const int* data = piece->draw_data;

	int y;
	for (y = 0; y < piece->h; ++y) {
		int x;
		for (x = 0; x < piece->w; ++x) {
			const int nx = piece->h - y - 1;
			buffer[x * piece->h + nx] = data[y * piece->w + x];
		}
	}

	int temp = piece->w;
	piece->w = piece->h;
	piece->h = temp;

	free(data);
	piece->draw_data = buffer;
}

int draw_score(tetris_context_t* ctx) {
	double bw, bh;

	query_board_size(ctx, &bw, &bh);

	if (ctx->font == NULL) {
		ctx->font = TTF_OpenFont(FONT_NAME, 24);

		if (ctx->font == NULL) {
			puts(TTF_GetError());
			return -1;
		}
	}

	SDL_Color color = { 255, 255, 255 };

	SDL_Surface* surface = TTF_RenderText_Solid(ctx->font, "I SAAAAAAAAAAY", color);

	SDL_Texture* text_texture = SDL_CreateTextureFromSurface(ctx->renderer, surface);

	SDL_FRect dst;
	dst.h = 60;
	dst.w = 700;
	dst.x = bw + 20;
	dst.y = 10;

	SDL_RenderCopyF(ctx->renderer, text_texture, NULL, &dst);

	SDL_FreeSurface(surface);
	SDL_DestroyTexture(text_texture);

    surface = TTF_RenderText_Solid(ctx->font, "HEY            ", color);

    text_texture = SDL_CreateTextureFromSurface(ctx->renderer, surface);

    dst.h = 60;
    dst.w = 700;
    dst.x = bw + 20;
    dst.y = 70;

    SDL_RenderCopyF(ctx->renderer, text_texture, NULL, &dst);

    SDL_FreeSurface(surface);
    SDL_DestroyTexture(text_texture);

    surface = TTF_RenderText_Solid(ctx->font, "HEY              ", color);

    text_texture = SDL_CreateTextureFromSurface(ctx->renderer, surface);

    dst.h = 60;
    dst.w = 700;
    dst.x = bw + 20;
    dst.y = 130;

    SDL_RenderCopyF(ctx->renderer, text_texture, NULL, &dst);

    SDL_FreeSurface(surface);
    SDL_DestroyTexture(text_texture);

    surface = TTF_RenderText_Solid(ctx->font, "HEY START DASH        ", color);

    text_texture = SDL_CreateTextureFromSurface(ctx->renderer, surface);

    dst.h = 60;
    dst.w = 700;
    dst.x = bw + 20;
    dst.y = 200;

    SDL_RenderCopyF(ctx->renderer, text_texture, NULL, &dst);

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

void game_update_title(tetris_context_t* ctx) {
	static char buffer[1024];
	*buffer = 0;

	double framerate = 1000.0 / (ctx->last_frame_duration * 1000);

	snprintf(buffer, sizeof(buffer), "Not Tetris - %dx%d - %.2f FPS", ctx->w_width, ctx->w_height, framerate);
	SDL_SetWindowTitle(ctx->window, buffer);
}

int game_run(tetris_context_t* ctx, game_loop_fn_t game_update) {
	int status_code, quit = 0;

	const int target_ticks = (1000 / ctx->target_framerate);

	game_loop_fn_t game_loop_functions[] = { game_collect_events, game_update, game_draw };

	uint64_t frame_ticks = SDL_GetTicks();

	int i;
	for (i = 0; i < sizeof game_loop_functions / sizeof(*game_loop_functions); ++i) {
		game_loop_fn_t fun = game_loop_functions[i];
		status_code = fun(ctx);
		if (status_code != 0) {
			quit = 1;
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

	return quit;
}