#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <SDL2/SDL.h>

int main(int argc, char** argv) {
	if (SDL_Init(SDL_INIT_VIDEO) != 0) {
		puts("Failed to initialize SDL");
		return EXIT_FAILURE;
	}

	int pos = SDL_WINDOWPOS_UNDEFINED;
	SDL_Window* window = SDL_CreateWindow("Test window", pos, pos, 800, 600, SDL_WINDOW_SHOWN);

	SDL_Delay(2500);

	SDL_DestroyWindow(window);
	SDL_Quit();

	return EXIT_SUCCESS;
}