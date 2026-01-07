#ifndef DEVSTUDIO_INPUT_H
#define DEVSTUDIO_INPUT_H

#include <SDL3/SDL.h>

typedef struct Button{
	char *labelText;
	SDL_FRect rect;
	void (*pressed)(struct Button*);
	bool enabled, visible, hover, down;
} Button;

bool updateButton(Button* item);
void drawButton(Button* item);

void buttonAddObject(Button* item);

void StudioHandleKeys();

#endif