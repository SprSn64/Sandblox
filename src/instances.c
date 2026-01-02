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

#define OBJLIST_HUD_POS_X 0
#define OBJLIST_HUD_POS_Y 32

void updateObject(DataObj* item, int nodeDepth, int *idCount){
	int i = (*idCount)++;
	drawText(renderer, fontTex, item->name, 32, OBJLIST_HUD_POS_X + (nodeDepth * 24), OBJLIST_HUD_POS_Y + i * 16, 16, 16, 12);
	if (item->class) {
		if (item->class->update) item->class->update(item);
		if (item->class->draw) item->class->draw(item);
	}

	DataObj* child = item->child;
	while (child) {
		DataObj *next = child->next;
		updateObject(child, nodeDepth + 1, idCount);
		child = next;
	}
}

DataObj* newObject(DataObj* parent, DataType* class){
	DataObj *newObj = calloc(1, sizeof(DataObj)); 
	if (parent == NULL) parent = &gameHeader;
	newObj->parent = parent;
	newObj->prev = NULL;
	newObj->next = parent->child;
	if (parent->child) {
		parent->child->prev = newObj;
	}
	parent->child = newObj;

	newObj->pos = (Vector3){0,0,0};
	newObj->scale = (Vector3){1,1,1};
	newObj->rot = (Vector3){0,0,0};
	newObj->colour = (CharColour){255, 255, 255};
	newObj->name = class->name;
	newObj->class = class;
	newObj->values = NULL;

	printf("Created new object of type '%s'.\n", class->name, newObj->name);
	
	return newObj;
}

bool parentObject(DataObj* child, DataObj* parent){
	if(parent->child == NULL){
		parent->child = child;
		//printf("%s -> %s\n", parent->name, parent->child->name);
		return 0;
	}

	DataObj* loopItem = parent->child;
	Uint32 loopCount = 1;
	for(Uint32 i = 0; i < loopCount; i++){
		if(loopItem->next == NULL) continue;
		loopItem = loopItem->next;
		loopCount++;
	}

	child->parent = parent;
	loopItem->next = child;
	child->prev = child;

	//printf("%s -> %s\n", parent->name, loopItem->next->name);

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
	object->pos.x += ((SDL_cos(currentCamera.rot.y) * (keyList[KEYBIND_D].down - keyList[KEYBIND_A].down)) + (SDL_sin(currentCamera.rot.y) * (keyList[KEYBIND_S].down - keyList[KEYBIND_W].down))) * 2 * deltaTime;
	object->pos.y += (keyList[KEYBIND_SPACE].down - keyList[KEYBIND_SHIFT].down) * 2 * deltaTime;
	object->pos.z += ((-SDL_sin(currentCamera.rot.y) * (keyList[KEYBIND_D].down - keyList[KEYBIND_A].down)) + (SDL_cos(currentCamera.rot.y) * (keyList[KEYBIND_S].down - keyList[KEYBIND_W].down))) * 2 * deltaTime;

	//object->pos.y = SDL_cos(timer) / 2 + 2;
	//currentCamera.pos = (Vector3){object->pos.x - SDL_cos(currentCamera.rot.y) * 4, object->pos.y + 2, object->pos.z - SDL_sin(currentCamera.rot.y) * 4};
}

void playerDraw(DataObj* object){
	drawCube((Vector3){object->pos.x - 1, object->pos.y + 4, object->pos.z - 1}, (Vector3){2, 4, 2}, (SDL_FColor){1, 1, 1, 1});
	drawBillboard(playerTex, (SDL_FRect){0, 0, 128, 128}, object->pos, (SDL_FPoint){8, 16}, (SDL_FPoint){4, 4});
}

DataType playerClass = {"Player\0", 0, NULL, playerUpdate, playerDraw};
DataType fuckingBeerdrinkerClass = {"beer drinker\0", 0, NULL, NULL, NULL};