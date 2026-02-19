#include <structs.h>
#include "input.h"
#include "pages.h"

#include <SDL3/SDL.h>
#include <SDL3_image/SDL_image.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

extern SDL_Window *window;
extern SDL_Renderer *renderer;

extern Font defaultFont;
void drawText(SDL_Renderer *renderLoc, Font *textFont, char* text, short posX, short posY, float scale, SDL_FColor colour);

Uint32 currPage = PAGE_GAME;
Page pageList[PAGE_MAX];

Button* sidePanelButtons = NULL;
Page sidePanel = {NULL, 3};

void updatePage(SDL_Renderer* render, Page* item){
	for(Uint32 i=0; i<item->buttonCount; i++){
		updateButton(&item->buttonList[i]);
		drawButton(render, &item->buttonList[i]);
	}
}

void initPages(){
	Button* sidePanelButtons = malloc(sizeof(Button) * sidePanel.buttonCount);
	sidePanelButtons[0] = (Button){"GAME", (SDL_FRect){2, 34, 64, 32}, buttonSetPage, true, true, false, false};
	sidePanelButtons[1] = (Button){"AVATAR", (SDL_FRect){2, 34*2, 64, 32}, buttonSetPage, true, true, false, false};
	sidePanelButtons[2] = (Button){"SETTINGS", (SDL_FRect){2, 34*3, 64, 32}, buttonSetPage, true, true, false, false};
	sidePanel.buttonList = sidePanelButtons;

	Button* gamePageButtons = malloc(sizeof(Button) * 2);
	gamePageButtons[0] = (Button){"Launch Sandblox", (SDL_FRect){66, 446, 572, 32}, buttonLaunch, true, true, false, false};
	gamePageButtons[1] = (Button){"select client dir", (SDL_FRect){510, 2, 128, 16}, buttonSelectClient, true, true, false, false};

	pageList[PAGE_GAME] = (Page){gamePageButtons, 2};
}