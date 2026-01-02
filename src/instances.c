#include <SDL3/SDL.h>
#include <SDL3_image/SDL_image.h>

#include <stdio.h>
#include <string.h>
#include <math.h>
#include <stdlib.h>

#include <structs.h>
#include "instances.h"
#include "renderer.h"

extern SDL_Renderer *renderer;
extern SDL_Texture *fontTex;

extern DataObj* playerObj;

DataObj gameHeader = {(Vector3){0, 0, 0}, (Vector3){1, 1, 1}, (Vector3){0, 0, 0}, (CharColour){0, 0, 0}, "Workspace", NULL, NULL, NULL, NULL, NULL, NULL};

void drawObjList(int posX, int posY){
	DataObj* loopItem = &gameHeader;
	Uint32 loopCount = 1;
	Uint16 offs = 0;
	for(Uint32 i = 0; i < loopCount; i++){
		SDL_SetRenderDrawColor(renderer, 64, 192, 24, SDL_ALPHA_OPAQUE);
		SDL_RenderFillRect(renderer, &(SDL_FRect){posX, posY + i * 16, 16, 16});
		drawText(renderer, fontTex, loopItem->name, 32, posX, posY + (i + offs) * 16, 16, 16, 12);
		if(loopItem->firstChild != NULL){
			offs++;
			drawText(renderer, fontTex, loopItem->firstChild->name, 32, posX, posY + (i + offs) * 16, 16, 16, 12);
		}
		if(loopItem->nextItem != NULL){
			loopItem = loopItem->nextItem;
		}else{
			break;
		}
		loopCount++;
	}
}

Uint8 loopUpdate(DataObj* item){
	//i have no idea what im doing
	DataObj* currItem = item;
	for(;;){
		updateObject(currItem);
		//printf("->%s\n", currItem->name);
		if(currItem->firstChild != NULL){
			loopUpdate(currItem->firstChild);
		}
		if(currItem->nextItem != NULL){
			currItem = currItem->nextItem;
		}else{
			break;
		}
	}
	return 0;
}

void updateObject(DataObj* item){
	//printf("Updating %s...\n", item->name);
	if(item->class != NULL){
		if (item->class->update) item->class->update(item);
		if (item->class->draw) item->class->draw(item);
	}
}

void updateObjects(DataObj* header){
	printf("%s\n", header->name);
}

DataObj* newObject(DataType* class){
	DataObj *newObj = calloc(1, sizeof(DataObj)); 
	newObj->pos = (Vector3){0,0,0};
	newObj->scale = (Vector3){1,1,1};
	newObj->rot = (Vector3){0,0,0};
	newObj->colour = (CharColour){255, 255, 255};
	newObj->name = class->name;
	newObj->class = class;
	newObj->values = NULL;
	newObj->prevItem, newObj->nextItem, newObj->parent, newObj->firstChild = NULL, NULL, NULL, NULL;
	parentObject(newObj, &gameHeader);

	printf("Created new object of type '%s' with name '%s'.\n", class->name, newObj->name);
	
	return newObj;
}

bool parentObject(DataObj* child, DataObj* parent){
	if(parent->firstChild == NULL){
		parent->firstChild = child;
		//printf("%s -> %s\n", parent->name, parent->firstChild->name);
		return 0;
	}
	
	DataObj* loopItem = parent->firstChild;
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

extern SDL_Texture *playerTex;

void playerUpdate(DataObj* object){
	//object->pos.x += (keyList[KEYBIND_D].down - keyList[KEYBIND_A].down) * 2 * deltaTime;
	//object->pos.z += (keyList[KEYBIND_S].down - keyList[KEYBIND_W].down) * 2 * deltaTime;

	object->pos.y = 2;
	//currentCamera.pos = object->pos;
}

void playerDraw(DataObj* object){
	drawBillboard(playerTex, (SDL_FRect){0, 0, 128, 128}, object->pos, (SDL_FPoint){8, 16}, (SDL_FPoint){4, 4});
}

DataType playerClass = {"Player\0", 0, NULL, playerUpdate, playerDraw};
DataType fuckingBeerdrinkerClass = {"beer drinker\0", 0, NULL, NULL, NULL};