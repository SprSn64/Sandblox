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

DataObj *focusObject = NULL;
SDL_MouseButtonFlags mouseState;
SDL_FPoint mousePos;

ButtonMap mouseButtons[3];

void drawObjectList(DataObj* item, int nodeDepth, int *idCount);
void drawObjectProperties(DataObj* item, int posY);

void initStudio(){
	//printf("Studio Initiated\n");
	if(!client.studio){printf("Studio not enabled!\n"); return;}
	
	if(!SDL_CreateWindowAndRenderer("Studio", studioWindowScale.x, studioWindowScale.y, SDL_WINDOW_UTILITY, &studioWindow, &studioRenderer)){
		printf("Error loading studio window - %s\n", SDL_GetError()); 
		return;
	}
	studioActive = true;
	SDL_SetWindowParent(studioWindow, window);
	//SDL_SetWindowMinimumSize(studioWindow, 320, 240);
	SDL_SetRenderVSync(studioRenderer, 1);
	
	mouseButtons[0].code = SDL_BUTTON_LMASK; mouseButtons[1].code = SDL_BUTTON_MMASK; mouseButtons[2].code = SDL_BUTTON_RMASK;
}

void updateStudio(){
	if(!studioActive) return;
	SDL_GetWindowSize(studioWindow, &studioWindowScale.x, &studioWindowScale.y);
	
	SDL_SetRenderDrawColor(studioRenderer, 148, 150, 152, 255);
	SDL_RenderClear(studioRenderer);
	
	mouseState = SDL_GetMouseState(&mousePos.x, &mousePos.y);
	for(int i=0; i<3; i++){
		mouseButtons[i].down = (SDL_GetWindowFlags(studioWindow) & SDL_WINDOW_INPUT_FOCUS && (mouseState & mouseButtons[i].code));
	}
	
	int idCounter = 0;
	drawObjectList(client.gameWorld->headObj, 0, &idCounter);
	
	drawObjectProperties(focusObject, 160);
	
	SDL_RenderPresent(studioRenderer);
}

void drawObjectList(DataObj* item, int nodeDepth, int *idCount){
	int i = (*idCount)++;
	if(mousePos.y >= 3 + i * 8 && mousePos.y <= 9 + i * 8 && mouseButtons[0].down){
		focusObject = item;
		goto focusSkip;
	}
	if(focusObject == item){
		focusSkip:
		SDL_SetRenderDrawColor(studioRenderer, 64, 192, 24, SDL_ALPHA_OPAQUE);
		SDL_RenderFillRect(studioRenderer, &(SDL_FRect){0, 2 + i * 8, 96, 8});
	}
	SDL_SetRenderDrawColor(studioRenderer, 255, 255, 255, 255);
	SDL_RenderDebugText(studioRenderer, 2/**/ + (nodeDepth * 12), 2/**/ + i * 8, item->name);
	DataObj* child = item->child;
	while (child) {
		DataObj *next = child->next;
		drawObjectList(child, nodeDepth + 1, idCount);
		child = next;
	}
}

void drawObjectProperties(DataObj* item, int posY){
	//excuse the slop
	char string[256];
	
	sprintf(string, "Name: %s", item->name);
	SDL_RenderDebugText(studioRenderer, 2, posY, string);
	sprintf(string, "Class: %s", item->classData->name);
	SDL_RenderDebugText(studioRenderer, 2, posY + 8, string);
	sprintf(string, "Position: %.2f, %.2f, %.2f", item->pos.x, item->pos.y, item->pos.z);
	SDL_RenderDebugText(studioRenderer, 2, posY + 16, string);
	sprintf(string, "Rotation: %d, %d, %d", (int)(item->rot.x * RAD2DEG), (int)(item->rot.y * RAD2DEG), (int)(item->rot.z * RAD2DEG));
	SDL_RenderDebugText(studioRenderer, 2, posY + 24, string);
	sprintf(string, "Scale: %.2f, %.2f, %.2f", item->scale.x, item->scale.y, item->scale.z);
	SDL_RenderDebugText(studioRenderer, 2, posY + 32, string);
}