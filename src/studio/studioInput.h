#ifndef DEVSTUDIO_INPUT_H
#define DEVSTUDIO_INPUT_H

#include <SDL3/SDL.h>
#include <structs.h>

typedef enum studioKeybinds{
	STUDIOKEYBIND_DELETE, 
	STUDIOKEYBIND_Z, STUDIOKEYBIND_X, STUDIOKEYBIND_C, STUDIOKEYBIND_V, STUDIOKEYBIND_B, STUDIOKEYBIND_N,
	STUDIOKEYBIND_D,
	STUDIOKEYBIND_MAX
} studioKeybinds;

typedef enum buttonInputType{
	INPUTTYPE_BUTTON,
	INPUTTYPE_DROPDOWN,
	INPUTTYPE_TEXT,
	INPUTTYPE_NUMBER, //click and drag left/right to change value
} buttonInputType;

typedef enum buttonColourEnum{
	BUTTONCOLOUR_DEFAULT,
	BUTTONCOLOUR_HOVER,
	BUTTONCOLOUR_PRESSED,
	BUTTONCOLOUR_DISABLED,
	BUTTONCOLOUR_INPUT,
	BUTTONCOLOUR_MAX, //keep at end of list
} buttonColourEnum;

typedef struct Button{
	char *labelText;
	SDL_FRect rect;
	Uint8 buttonType;
	void (*pressed)(struct Button*);
	bool enabled, visible, hover, down;
	SDL_Texture* image;
	SDL_FRect* imageSrc;
} Button;

bool updateButton(Button* item);
void drawButton(SDL_Renderer* render, Button* item);
void updateAndDrawButton(SDL_Renderer* render, Button* item);

void buttonAddObject(Button* item);
void buttonRemoveObject(Button* item);
void buttonLoadMap(Button* item);
void buttonSaveMap(Button* item);
void buttonPauseGame(Button* item);
void buttonSetTool(Button* item);

void StudioHandleKeys();

#endif