#include "studioMain.h"
#include "studioInput.h"
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

SDL_Texture *classIconTex = NULL;

SDL_Point studioWindowScale = {240, 320};

DataObj *focusObject = NULL;
extern SDL_MouseButtonFlags mouseState;
extern SDL_FPoint mousePos;
extern ButtonMap stuMouseButtons[3];

extern ButtonMap stuKeyList[5];

void drawObjectList(DataObj* item, int nodeDepth, int *idCount);
void drawObjectProperties(DataObj* item, int posY);

//TODO: Make button list thing for less ugly looking button implementations

Button addObjButton = {"+", (SDL_FRect){224, 304, 16, 16}, buttonAddObject, true, true, false, false};
Button removeObjButton = {"-", (SDL_FRect){206, 304, 16, 16}, buttonRemoveObject, true, true, false, false};
Button fileButton = {"File", (SDL_FRect){0, 0, 48, 16}, NULL, false, true, false, false};

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
	
	stuMouseButtons[0].code = SDL_BUTTON_LMASK; stuMouseButtons[1].code = SDL_BUTTON_MMASK; stuMouseButtons[2].code = SDL_BUTTON_RMASK;
	
	classIconTex = IMG_LoadTexture(studioRenderer, "assets/textures/classicons.png");
}

void updateStudio(){
	if(!studioActive) return;
	SDL_GetWindowSize(studioWindow, &studioWindowScale.x, &studioWindowScale.y);
	
	SDL_SetRenderDrawColor(studioRenderer, 148, 150, 152, 255);
	SDL_RenderClear(studioRenderer);
	
	for(int i=0; i<3; i++){
		stuMouseButtons[i].down = (SDL_GetWindowFlags(studioWindow) & SDL_WINDOW_INPUT_FOCUS && (mouseState & stuMouseButtons[i].code));
	}
	SDL_SetCursor(SDL_CreateSystemCursor(SDL_SYSTEM_CURSOR_DEFAULT));
	
	int idCounter = 0;
	drawObjectList(client.gameWorld->headObj, 0, &idCounter);
	
	updateButton(&addObjButton); updateButton(&removeObjButton);
	drawButton(&addObjButton); drawButton(&removeObjButton); drawButton(&fileButton);
	
	drawObjectProperties(focusObject, 240);
	
	SDL_RenderPresent(studioRenderer);
}

void drawObjectList(DataObj* item, int nodeDepth, int *idCount){
	int i = (*idCount)++;
	if(mousePos.y >= 19 + i * 16 && mousePos.y <= 31 + i * 16 && stuMouseButtons[0].down){
		focusObject = item;
		goto focusSkip;
	}
	if(focusObject == item){
		focusSkip:
		SDL_SetRenderDrawColor(studioRenderer, 64, 192, 24, SDL_ALPHA_OPAQUE);
		SDL_RenderFillRect(studioRenderer, &(SDL_FRect){0, 17 + i * 16, 256, 16});
	}
	SDL_SetRenderDrawColor(studioRenderer, 255, 255, 255, 255);
	SDL_RenderDebugText(studioRenderer, 18/**/ + (nodeDepth * 24), 18/**/ + i * 16, item->name);
	
	SDL_FRect iconRect = {(item->classData->id % 16) * 16, (int)floor((float)item->classData->id / 16) * 16 % 16, 16, 16};
	SDL_FRect iconPos = {2 + (nodeDepth * 24), 18/**/ + i * 16, 16, 16};
	SDL_RenderTexture(studioRenderer, classIconTex, &iconRect, &iconPos);
	
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
	
	SDL_SetRenderDrawColor(studioRenderer, 255, 255, 255, 255);
	
	if(!item){
		SDL_RenderDebugText(studioRenderer, 2, posY, "No object selected!"); return;
	}
	
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