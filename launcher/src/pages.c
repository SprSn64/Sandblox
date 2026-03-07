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
SDL_Texture *newTexture(char* path, SDL_ScaleMode scaleMode);
void drawText(SDL_Renderer *renderLoc, Font *textFont, char* text, short posX, short posY, float scale, SDL_FColor colour);
extern char** langStrings;

extern char* playerName;

Uint32 currPage = PAGE_GAME;
Page pageList[PAGE_MAX];

Button* sidePanelButtons = NULL;
Page sidePanel = {NULL, 5};

SDL_Texture* buttonTex = NULL;

void updatePage(SDL_Renderer* render, Page* item){
	for(Uint32 i=0; i<item->buttonCount; i++){
		updateButton(&item->buttonList[i]);
		drawButton(render, &item->buttonList[i]);
	}
}

void initPages(){
	buttonTex = newTexture("assets/buttonicons.png", SDL_SCALEMODE_NEAREST);

	Button* sidePanelButtons = malloc(sizeof(Button) * sidePanel.buttonCount);
	sidePanelButtons[0] = newLableButton(buttonSetPage, "GAME", (SDL_FRect){2, 34, 64, 32});
	sidePanelButtons[1] = newLableButton(buttonSetPage, "AVATAR", (SDL_FRect){2, 68, 64, 32});
	sidePanelButtons[2] = newLableButton(buttonSetPage, "SETTINGS", (SDL_FRect){2, 102, 64, 32});
	sidePanelButtons[3] = newImageButton(buttonOpenLink, buttonTex, (SDL_FRect){0, 448, 32, 32}, (SDL_FRect){0, 0, 32, 32});
	sidePanelButtons[4] = newImageButton(buttonOpenLink, buttonTex, (SDL_FRect){32, 448, 32, 32}, (SDL_FRect){32, 0, 32, 32});
	sidePanel.buttonList = sidePanelButtons;

	Button* gamePageButtons = malloc(sizeof(Button) * 3);
	gamePageButtons[0] = newLableButton(buttonLaunch, "Launch Sandblox", (SDL_FRect){66, 446, 572, 32});
	gamePageButtons[1] = newLableButton(buttonSelectClient, "Set Client", (SDL_FRect){510, 2, 128, 16});
	gamePageButtons[2] = newLableButton(buttonRefreshMaps, "Refresh Map List", (SDL_FRect){380, 2, 128, 16});
	pageList[PAGE_GAME] = (Page){gamePageButtons, 3};

	Button* avatarPageButtons = malloc(sizeof(Button) * 1);
	avatarPageButtons[0] = newTextboxButton(playerName, "Username", (SDL_FRect){96, 346, 128, 16});
	pageList[PAGE_AVATAR] = (Page){avatarPageButtons, 1};

	Button* settingsPageButtons = malloc(sizeof(Button) * 1);
	settingsPageButtons[0] = newLableButton(NULL, "KILL EVERYONE!!!", (SDL_FRect){204, 208, 128, 16});
	pageList[PAGE_SETTINGS] = (Page){settingsPageButtons, 1};
}