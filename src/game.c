#include "engine.h"

#include <stdio.h>

#include <SDL2/SDL.h>

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
					if (piece->x > 1) {
						piece->x -= 1;
					}
					break;
				case SDLK_RIGHT:
				case SDLK_d:
					if (piece->x < BOARD_COLUMNS - 3) {
						piece->x += 1;
					}
					break;
				case SDLK_UP:
				case SDLK_w:
					if (piece->y > 1) {
						piece->y -= 1;
					}
					break;
				case SDLK_DOWN:
				case SDLK_s:
					if (piece->y < BOARD_ROWS - 3) {
						piece->y += 1;
					}
					break;
				case SDLK_r:
					rotate_piece(piece);
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

int start_game(void) {
	int status_code = 0;

	tetris_context_t* ctx = context_create();
	while ((status_code = game_run(ctx, game_update)) == 0);
	context_destroy(ctx);

	return status_code;
}