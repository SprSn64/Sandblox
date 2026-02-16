#ifndef INPUT_H
#define INPUT_H

#include <SDL3/SDL.h>

typedef struct Button{
	char *labelText;
	SDL_FRect rect;
	void (*pressed)(struct Button*);
	bool enabled, visible, hover, down;
} Button;

bool updateButton(Button* item);
void drawButton(SDL_Renderer* render, Button* item);

void buttonLaunch(Button* item);
void buttonSelectClient(Button* item);

#endif