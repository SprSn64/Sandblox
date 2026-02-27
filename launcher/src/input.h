#ifndef INPUT_H
#define INPUT_H

#include <SDL3/SDL.h>

typedef struct Button{
	char *labelText;
	SDL_FRect rect;
	void (*pressed)(struct Button*);
	bool enabled, visible, hover, down; //might kill the invisible value
	SDL_Texture* image;
	SDL_FRect* imageSrc;
} Button;

Button newLableButton(void* func, char* text, SDL_FRect rect);
Button newImageButton(void* func, SDL_Texture* image, SDL_FRect dest, SDL_FRect source);
bool updateButton(Button* item);
void drawButton(SDL_Renderer* render, Button* item);

void buttonLaunch(Button* item);
void buttonSelectClient(Button* item);
void buttonSetPage(Button* item);
void buttonOpenLink(Button* item);

#endif