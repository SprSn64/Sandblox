#ifndef INPUT_H
#define INPUT_H

#include <SDL3/SDL.h>

typedef enum buttonInputType{
	INPUTTYPE_BUTTON,
	INPUTTYPE_DROPDOWN,
	INPUTTYPE_TEXT,
	INPUTTYPE_NUMBER, //click and drag left/right to change value
	INPUTTYPE_COLOUR
} buttonInputType;

typedef struct Button{
	char *labelText;
	SDL_FRect rect;
	Uint8 buttonType;
	void (*pressed)(struct Button*);
	bool enabled, hover, down;
	void *target;
	SDL_Texture* image;
	SDL_FRect* imageSrc;
} Button;

Button newLableButton(void* func, char* text, SDL_FRect rect);
Button newImageButton(void* func, SDL_Texture* image, SDL_FRect dest, SDL_FRect source);
Button newTextboxButton(void* target, char* text, SDL_FRect rect);
bool updateButton(Button* item);
void drawButton(SDL_Renderer* render, Button* item);

void buttonLaunch(Button* item);
void buttonSelectClient(Button* item);
void buttonSetPage(Button* item);
void buttonOpenLink(Button* item);
void buttonRefreshMaps(Button* item);

#endif