#include "engine.h"

#include <SDL2/SDL.h>

#define AXIS_X (0)
#define AXIS_Y (1)

static int collides_x(const tetris_board_t *board, int x_offset) {
    int collides = 0;

    const tetris_piece_t *piece = board->current_piece;

    int row;
    for(row = 0; row < piece->h; ++row) {
        // Find first block from each edge

        if(x_offset > 0) {
            int col;
            for(col = piece->w - 1; col >= 0; --col) {
                if(piece->draw_data[row * piece->w + col] != 0) {
                    int xpos = piece->x + col + x_offset;

                    if(board_get_cell(board, xpos, piece->y + row) != g_tetris_colors[COLOR_NONE]) {
                        collides = 1;
                    }
                    break;
                }
            }
        } else if(x_offset < 0) {
            int col;
            for(col = 0; col < piece->h; ++col) {
                if(piece->draw_data[row * piece->w + col] != 0) {
                    int xpos = piece->x + col + x_offset;

                    if(board_get_cell(board, xpos, piece->y + row) != g_tetris_colors[COLOR_NONE]) {
                        collides = 1;
                    }
                    break;
                }
            }
        }

        if(collides) {
            break;
        }
    }

    return collides;
}

static int collides_y(const tetris_board_t *board, int y_offset) {
    int collides = 0;

    const tetris_piece_t *piece = board->current_piece;

    int col;
    for(col = 0; col < piece->w; ++col) {
        int row;
        for(row = piece->h - 1; row >= 0; --row) {
            if(piece->draw_data[row * piece->w + col] != 0) {
                int ypos = piece->y + row + y_offset;
                int xpos = piece->x + col;

                if(board_get_cell(board, xpos, ypos) != g_tetris_colors[COLOR_NONE]) {
                    collides = 1;
                }
                break;
            }
        }

        if(collides) {
            break;
        }
    }

    return collides;
}

static void game_move_piece(tetris_context_t *ctx, int axis, int amount) {
    if(ctx->board.current_piece != NULL) {
        if(axis == AXIS_X) {
            if(collides_x(&ctx->board, amount) == 0) {
                ctx->board.current_piece->x += amount;
            }
        } else if(axis == AXIS_Y) {
            if(collides_y(&ctx->board, amount) == 0) {
                ctx->board.current_piece->y += amount;
            }
        }
    }
}

static int game_check_input(tetris_context_t *ctx) {
    int i;
    for (i = 0; i < ctx->event_stack_top; ++i) {
        tetris_event_t* ev = &ctx->event_stack[i];
        if (ev->kind == EVENT_KEYDOWN) {
            switch (ev->data) {
                case SDLK_LEFT:
                case SDLK_a:
                    game_move_piece(ctx, AXIS_X, -1);
                    break;
                case SDLK_RIGHT:
                case SDLK_d:
                    game_move_piece(ctx, AXIS_X, 1);
                    break;
                case SDLK_DOWN:
                case SDLK_s:
                    game_move_piece(ctx, AXIS_Y, 1);
                    break;
                case SDLK_r:
                    game_rotate_piece(&ctx->board);
                    break;
                default:
                    break;
            }
        }
    }
    ctx->event_stack_top = 0;

    return 0;
}

int game_update(tetris_context_t* ctx) {
	int status_code;

    if(ctx->board.current_piece == NULL || collides_y(&ctx->board, 1)) {
        board_fixate_current_piece(&ctx->board);
        board_spawn_piece(&ctx->board);
    }

	if((status_code = game_check_input(ctx)) != 0) {
	    return status_code;
	}

	return 0;
}

int start_game(void) {
	int status_code;

	tetris_context_t* ctx = context_create();
	while ((status_code = game_run(ctx, game_update)) == 0);
	context_destroy(ctx);

	return status_code;
}