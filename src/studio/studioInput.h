#ifndef DEVSTUDIO_INPUT_H
#define DEVSTUDIO_INPUT_H

#include <SDL3/SDL.h>

typedef struct{
	char *labelText;
	SDL_FRect rect;
	void (*pressed)(void);
	bool enabled, visible, hover, down;
} Button;

#endif