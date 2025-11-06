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

DataObj gameHeader = {(Vector3){0, 0, 0}, (Vector3){1, 1, 1}, (Vector3){0, 0, 0}, (CharColour){0, 0, 0}, "beer licker"};
DataObj itemB = {(Vector3){0, 0, 0}, (Vector3){1, 1, 1}, (Vector3){0, 0, 0}, (CharColour){0, 0, 0}, "beer drinker"};

void drawObjList(int posX, int posY){
	gameHeader.nextItem = &itemB;
	
	DataObj* loopItem = &gameHeader;
	Uint32 loopCount = 1;
	for(int i = 0; i < loopCount; i++){
		drawText(renderer, fontTex, loopItem->name, 32, posX, posY + i * 16, 16, 16, 12);
		if(loopItem->nextItem != NULL){
			loopItem = loopItem->nextItem;
			loopCount++;
		}
	}
}

