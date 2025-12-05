#include <SDL3/SDL.h>
#include <SDL3_image/SDL_image.h>

#include <stdio.h>
#include <string.h>
#include <math.h>

#include <structs.h>
#include "instances.h"
#include "renderer.h"

extern SDL_Renderer *renderer;
extern SDL_Texture *fontTex;

DataObj gameHeader = {(Vector3){0, 0, 0}, (Vector3){1, 1, 1}, (Vector3){0, 0, 0}, (CharColour){0, 0, 0}, "Workspace"};
DataObj itemB = {(Vector3){0, 0, 0}, (Vector3){1, 1, 1}, (Vector3){0, 0, 0}, (CharColour){0, 0, 0}, "beer drinker"};

void drawObjList(int posX, int posY){
	DataObj* loopItem = &gameHeader;
	Uint32 loopCount = 1;
	for(int i = 0; i < loopCount; i++){
		SDL_SetRenderDrawColor(renderer, 64, 192, 24, SDL_ALPHA_OPAQUE);
		SDL_RenderFillRect(renderer, &(SDL_FRect){posX, posY + i * 16, 16, 16});
		drawText(renderer, fontTex, loopItem->name, 32, posX, posY + i * 16, 16, 16, 12);
		if(loopItem->nextItem != NULL){
			loopItem = loopItem->nextItem;
			loopCount++;
		}
	}
}

DataObj* newObject(DataType* class){
	DataObj newObj = {(Vector3){0, 0, 0}, (Vector3){1, 1, 1}, (Vector3){0, 0, 0}, (CharColour){255, 255, 255}, class->name, class};
	
	return &newObj;
}

bool parentObject(DataObj* child, DataObj* parent){
	DataObj* loopItem = gameHeader.nextItem;
	return 0;
}

extern float timer;
extern double deltaTime;
extern Camera currentCamera;
extern KeyMap keyList[6];

extern SDL_Texture playerTex;

void playerUpdate(DataObj* object){
	object->pos.x += (keyList[3].down - keyList[2].down) * 2 * deltaTime;
	object->pos.z += (keyList[1].down - keyList[0].down) * 2 * deltaTime;
	//
	currentCamera.pos = object->pos;
}

void playerDraw(DataObj* object){
	drawBillboard(&playerTex, (SDL_FRect){0, 0, 128, 128}, (Vector3){0, 2, 0}, (SDL_FPoint){8, 16}, (SDL_FPoint){4, 4});
}

DataType playerClass = {"Player", 0, NULL, playerUpdate, playerDraw};