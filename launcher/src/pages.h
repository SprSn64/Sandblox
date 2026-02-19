#ifndef PAGES_H
#define PAGES_H

#include <SDL3/SDL.h>
#include "input.h"

typedef enum pageValues{
	PAGE_GAME, PAGE_AVATAR, PAGE_SETTINGS, PAGE_MAX
} pageValues;

typedef struct{
	Button* buttonList;
	Uint32 buttonCount;
} Page;

void updatePage(SDL_Renderer* render, Page* item);
void initPages();

#endif
