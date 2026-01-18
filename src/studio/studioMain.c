#include "studioMain.h"
#include "studioInput.h"
#include <structs.h>

#include <SDL3/SDL.h>
#include <SDL3_image/SDL_image.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#include "../math.h"

extern SDL_Window *window;
extern ClientData client;

SDL_Window *studioWindow = NULL;
SDL_Renderer *studioRenderer = NULL;
bool studioActive = false;

extern double deltaTime;
extern float timer;

extern float* defaultMatrix;

SDL_Texture *classIconTex = NULL;

SDL_Point studioWindowScale = {240, 320};

float objListScroll = 0;
Uint32 objListLength = 0;
SDL_Rect objListRect = {32, 16, 208, 240};

DataObj *focusObject = NULL;
extern SDL_MouseButtonFlags mouseState;
extern SDL_FPoint mousePos;
extern ButtonMap stuMouseButtons[3];

extern ButtonMap stuKeyList[5];
extern ButtonMap keyList[KEYBINDCOUNT];

void drawObjectList(DataObj* item, int nodeDepth, int *idCount);
void drawObjectProperties(DataObj* item, int posY);

//TODO: Make button list thing for less ugly looking button implementations

Button addObjButton = {"+", (SDL_FRect){224, 304, 16, 16}, buttonAddObject, true, true, false, false};
Button removeObjButton = {"-", (SDL_FRect){206, 304, 16, 16}, buttonRemoveObject, true, true, false, false};
Button fileButton = {"Load", (SDL_FRect){0, 0, 48, 16}, buttonLoadMap, true, true, false, false};
Button pauseButton = {"ll", (SDL_FRect){0, 304, 16, 16}, buttonPauseGame, true, true, false, false};

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

void studioCameraUpdate(Camera* cam);

void updateStudio(){
	if(!studioActive) return;
	SDL_GetWindowSize(studioWindow, &studioWindowScale.x, &studioWindowScale.y);
	
	StudioHandleKeys();
	
	SDL_SetRenderDrawColor(studioRenderer, 148, 150, 152, 255);
	SDL_RenderClear(studioRenderer);
	
	for(int i=0; i<3; i++){
		stuMouseButtons[i].down = (SDL_GetWindowFlags(studioWindow) & SDL_WINDOW_INPUT_FOCUS && (mouseState & stuMouseButtons[i].code));
		if(stuMouseButtons[i].down){
			if(!stuMouseButtons[i].pressCheck){
				stuMouseButtons[i].pressCheck = true;
				stuMouseButtons[i].pressed = true;
			}else{
				stuMouseButtons[i].pressed = false;
			}
		}else stuMouseButtons[i].pressCheck = false;
	}
	SDL_SetCursor(SDL_CreateSystemCursor(SDL_SYSTEM_CURSOR_DEFAULT));
	
	if(client.pause)
		studioCameraUpdate(client.gameWorld->currCamera);
	
	objListLength = 0;
	int idCounter = 0;
	drawObjectList(client.gameWorld->headObj, 0, &idCounter);
	
	//make this less shitty soon
	updateButton(&addObjButton); updateButton(&removeObjButton); updateButton(&fileButton); updateButton(&pauseButton);
	drawButton(&addObjButton); drawButton(&removeObjButton); drawButton(&fileButton); drawButton(&pauseButton);
	
	drawObjectProperties(focusObject, 240);
	
	SDL_RenderPresent(studioRenderer);
}

void drawObjectList(DataObj* item, int nodeDepth, int *idCount){	
	int i = (*idCount)++;
	float itemYOffset = (i - objListScroll) * 16;
	if(closest(itemYOffset - 8, 16) >= objListRect.h - 32) 
		return;
	
	if(itemYOffset < 0)
		goto listRenderSkip;
	
	if(between(mousePos.x, objListRect.x, objListRect.x + objListRect.w) && between(mousePos.y, objListRect.y + 3 + itemYOffset, objListRect.y + 15 + itemYOffset) && stuMouseButtons[0].pressed){
		if(item == focusObject){
			item->studioOpen = !item->studioOpen;
		}
		focusObject = item;
		goto focusSkip;
	}
	if(focusObject == item){
		focusSkip:
		SDL_SetRenderDrawColor(studioRenderer, 64, 192, 24, SDL_ALPHA_OPAQUE);
		SDL_RenderFillRect(studioRenderer, &(SDL_FRect){objListRect.x, objListRect.y + itemYOffset, objListRect.w, 16});
	}
	SDL_SetRenderDrawColor(studioRenderer, 255, 255, 255, 255);
	SDL_RenderDebugText(studioRenderer, objListRect.x + 18/**/ + (nodeDepth * 24), 20/**/ + itemYOffset, item->name);
	
	SDL_FRect iconRect = {(item->classData->id % 16) * 16, (int)floor((float)item->classData->id / 16) * 16 % 16, 16, 16};
	SDL_FRect iconPos = {objListRect.x + nodeDepth * 24, objListRect.y/**/ + itemYOffset, 16, 16};
	SDL_RenderTexture(studioRenderer, classIconTex, &iconRect, &iconPos);
	
	if(!item->studioOpen && item->child)
		SDL_RenderTexture(studioRenderer, classIconTex, &(SDL_FRect){245, 249, 11, 7}, &(SDL_FRect){objListRect.x + nodeDepth * 24, objListRect.y + itemYOffset, 11, 7});
	
	listRenderSkip:
	
	if(!item->studioOpen) return;
	
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
	
	SDL_RenderDebugText(studioRenderer, 2, posY + 40, "Colour: ");
	SDL_SetRenderDrawColor(studioRenderer, item->colour.r, item->colour.g, item->colour.b, 255); 
	SDL_RenderFillRect(studioRenderer, &(SDL_FRect){64, posY + 40, 24, 8});
	SDL_SetRenderDrawColor(studioRenderer, item->colour.r * item->colour.a / 255, item->colour.g * item->colour.a / 255, item->colour.b * item->colour.a / 255, 255); 
	SDL_RenderFillRect(studioRenderer, &(SDL_FRect){88, posY + 40, 24, 8});
}

void studioCameraUpdate(Camera* cam){
	//make camera move forwards in all axis, not just x and z
	float camSpeed = 2;
	
	Vector4 moveVec = {
		(keyList[KEYBIND_D].down - keyList[KEYBIND_A].down), 
		(keyList[KEYBIND_SPACE].down - keyList[KEYBIND_SHIFT].down), 
		(keyList[KEYBIND_S].down - keyList[KEYBIND_W].down), 
		0
	};
	
	float* camRotMatrix = rotateMatrix(defaultMatrix, cam->rot);
	moveVec = matrixMult(moveVec, camRotMatrix);
	
	cam->pos = (Vector3){cam->pos.x + moveVec.x * camSpeed * deltaTime, cam->pos.y + moveVec.y * camSpeed * deltaTime, cam->pos.z + moveVec.z * camSpeed * deltaTime};
	free(camRotMatrix);
}