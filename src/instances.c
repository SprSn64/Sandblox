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

extern DataObj* playerObj;

DataObj gameHeader = {(Vector3){0, 0, 0}, (Vector3){1, 1, 1}, (Vector3){0, 0, 0}, (CharColour){0, 0, 0}, "Workspace", NULL, NULL, NULL, NULL, NULL, NULL};
DataObj itemB = {(Vector3){0, 0, 0}, (Vector3){1, 1, 1}, (Vector3){0, 0, 0}, (CharColour){0, 0, 0}, "beer drinker", NULL, NULL, NULL, NULL, NULL, NULL};

void drawObjList(int posX, int posY){
	DataObj* loopItem = &gameHeader;
	Uint32 loopCount = 1;
	for(Uint32 i = 0; i < loopCount; i++){
		SDL_SetRenderDrawColor(renderer, 64, 192, 24, SDL_ALPHA_OPAQUE);
		SDL_RenderFillRect(renderer, &(SDL_FRect){posX, posY + i * 16, 16, 16});
		drawText(renderer, fontTex, loopItem->name, 32, posX, posY + i * 16, 16, 16, 12);
		if(loopItem->firstChild != NULL)drawText(renderer, fontTex, loopItem->firstChild->name, 32, posX + 256, posY + i * 16, 16, 16, 12); //temporary single child code
		
		//insert script for parented children here
		
		if(loopItem->nextItem == NULL) continue;
		loopItem = loopItem->nextItem;
		loopCount++;
	}
}

Uint8 parentChildLoop(DataObj* currObj, Uint16* yIndex, Uint16* depth){
	//i have no idea what im doing
	printf("%s, %ls, %ls\n", currObj->name, yIndex, depth);
	
	if(currObj->firstChild != NULL){
		parentChildLoop(currObj->firstChild, NULL, NULL);
	}
	if(currObj->nextItem != NULL){
		parentChildLoop(currObj->nextItem, NULL, NULL);
	}
	return 0;
}

void updateObjects(DataObj* header){
	printf("%s\n", header->name);
}

DataObj* newObject(DataType* class){
	DataObj newObj = {(Vector3){0, 0, 0}, (Vector3){1, 1, 1}, (Vector3){0, 0, 0}, (CharColour){255, 255, 255}, class->name, class, NULL, NULL, NULL, NULL, NULL};
	printf("Created new object of type '%s'.\n", class->name);
	
	return &newObj;
}

bool parentObject(DataObj* child, DataObj* parent){
	DataObj* loopItem = parent->firstChild;
	if(loopItem == NULL){
		parent->firstChild = child;
		//printf("%s -> %s\n", parent->name, parent->firstChild->name);
		return 0;
	}
	
	Uint32 loopCount = 1;
	for(Uint32 i = 0; i < loopCount; i++){
		if(loopItem->nextItem == NULL) continue;
		loopItem = loopItem->nextItem;
		loopCount++;
	}
	
	child->parent = parent;
	loopItem->nextItem = child;
	child->prevItem = child;
	
	//printf("%s -> %s\n", parent->name, loopItem->nextItem->name);
	
	return 0;
}

CollsionReturn* getCollision(CollisionHull* itemA, CollisionHull* itemB){
	/*if(collide is yes) then do'eth
		tell me the collision outputs then please
	  else*/
	return NULL;
	//end
}

//--------------------------------------------- temporarys until i put them into their own little file johnson

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
	drawBillboard(&playerTex, (SDL_FRect){0, 0, 128, 128}, object->pos, (SDL_FPoint){8, 16}, (SDL_FPoint){4, 4});
}

DataType playerClass = {"Player\0", 0, NULL, playerUpdate, playerDraw};