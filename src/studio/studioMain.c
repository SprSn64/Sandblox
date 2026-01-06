#include "studioMain.h"
#include <structs.h>

#include <SDL3/SDL.h>
#include <SDL3_image/SDL_image.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

extern SDL_Window *window;
extern ClientData client;

SDL_Window *studioWindow = NULL;
SDL_Renderer *studioRenderer = NULL;
bool studioActive = false;

SDL_Point studioWindowScale;

void drawObjectList(DataObj* item, int nodeDepth, int *idCount);

void initStudio(){
	//printf("Studio Initiated\n");
	if(!client.studio){printf("Studio not enabled!\n"); return;}
	
	studioWindow = SDL_CreateWindow("Studio", 240, 320, SDL_WINDOW_UTILITY);
	studioRenderer = SDL_CreateRenderer(studioWindow, NULL);
	if(!studioWindow){printf("Error loading studio window!\n"); return;}
	studioActive = true;
	SDL_SetWindowParent(studioWindow, window); //SDL_SetWindowModal(glWindow, true);
	//SDL_SetWindowMinimumSize(studioWindow, 320, 240);
}

void updateStudio(){
	if(!studioActive) return;
	SDL_GetWindowSize(studioWindow, &studioWindowScale.x, &studioWindowScale.y);
	
	SDL_SetRenderDrawColor(studioRenderer, 148, 150, 152, SDL_ALPHA_OPAQUE);
	SDL_RenderClear(studioRenderer);
	
	int idCounter = 0;
	drawObjectList(client.gameWorld->headObj, 0, &idCounter);
}

void drawObjectList(DataObj* item, int nodeDepth, int *idCount){
	int i = (*idCount)++;
	SDL_RenderDebugText(studioRenderer, 0/**/ + (nodeDepth * 24), 0/**/ + i * 16, item->name);
	DataObj* child = item->child;
	while (child) {
		DataObj *next = child->next;
		drawObjectList(child, nodeDepth + 1, idCount);
		child = next;
	}
}