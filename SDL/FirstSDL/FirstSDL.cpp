/*
Make sure you have SDL2.dll in C:/Windows/System32, or put it in your project.
you can download it from here http://www.libsdl.org/download-2.0.php
*/

#include "SDL.h"

int main(int argc, char* args[])
{
	// Start SDL
	SDL_Init(SDL_INIT_EVERYTHING);

	// Quit SDL
	SDL_Quit();

	return 0;
}