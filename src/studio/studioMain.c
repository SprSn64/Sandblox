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

SDL_Point studioWindowScale = {240, 320};

void drawObjectList(DataObj* item, int nodeDepth, int *idCount);

void initStudio(){
	//printf("Studio Initiated\n");
	if(!client.studio){printf("Studio not enabled!\n"); return;}
	
	if(!SDL_CreateWindowAndRenderer("Studio", studioWindowScale.x, studioWindowScale.y, SDL_WINDOW_UTILITY, &studioWindow, &studioRenderer)){
		printf("Error loading studio window - %s\n", SDL_GetError()); 
		return;
	}
	studioActive = true;
	SDL_SetWindowParent(studioWindow, window); //SDL_SetWindowModal(glWindow, true);
	//SDL_SetWindowMinimumSize(studioWindow, 320, 240);
}

void updateStudio(){
	if(!studioActive) return;
	SDL_GetWindowSize(studioWindow, &studioWindowScale.x, &studioWindowScale.y);
	
	SDL_SetRenderDrawColor(studioRenderer, 148, 150, 152, 255);
	SDL_RenderClear(studioRenderer);
	
	int idCounter = 0;
	SDL_SetRenderDrawColor(studioRenderer, 0, 0, 0, 255);
	drawObjectList(client.gameWorld->headObj, 0, &idCounter);
	
	SDL_RenderPresent(studioRenderer);
}

void drawObjectList(DataObj* item, int nodeDepth, int *idCount){
	int i = (*idCount)++;
	SDL_RenderDebugText(studioRenderer, 2/**/ + (nodeDepth * 12), 2/**/ + i * 8, item->name);
	DataObj* child = item->child;
	while (child) {
		DataObj *next = child->next;
		drawObjectList(child, nodeDepth + 1, idCount);
		child = next;
	}
}