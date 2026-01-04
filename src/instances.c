#include <SDL3/SDL.h>
#include <SDL3_image/SDL_image.h>

#include <stdio.h>
#include <string.h>
#include <math.h>
#include <stdlib.h>

#include <structs.h>
#include "instances.h"
#include "renderer.h"
#include "math.h" //causes "error: storage class specified for parameter..." for some reason
#include "loader.h"

extern SDL_Renderer *renderer;
extern SDL_Texture *fontTex;

extern DataObj* playerObj;

DataObj gameHeader = {(Vector3){0, 0, 0}, (Vector3){1, 1, 1}, (Vector3){0, 0, 0}, NULL, (CharColour){0, 0, 0, 255}, "Workspace", NULL, NULL, NULL, NULL, NULL, NULL};

#define OBJLIST_HUD_POS_X 0
#define OBJLIST_HUD_POS_Y 32

void updateObjects(DataObj* item, int nodeDepth, int *idCount, bool uord){ //uord = update or draw
	int i = (*idCount)++;
	item->transform = newMatrix();
	translateMatrix2(item->transform, (Vector3){item->pos.x, item->pos.y, item->pos.z});
	scaleMatrix2(item->transform, (Vector3){item->scale.x, item->scale.y, item->scale.z});
	//insert rotation here
	if (item->class) {
		if (item->class->update && !uord) item->class->update(item);
		if (item->class->draw && uord) item->class->draw(item);
	}
	free(item->transform);
	if(uord)drawText(renderer, fontTex, item->name, 32, OBJLIST_HUD_POS_X + (nodeDepth * 24), OBJLIST_HUD_POS_Y + i * 16, 16, 16, 12);
	DataObj* child = item->child;
	while (child) {
		DataObj *next = child->next;
		updateObjects(child, nodeDepth + 1, idCount, uord);
		child = next;
	}
}

void cleanupObjects(DataObj* item){
	DataObj* child = item->child;
	while (child) {
		DataObj *next = child->next;
		cleanupObjects(child);
		child = next;
	}
	free(item);
}

DataObj* newObject(DataObj* parent, DataType* class){
	DataObj *newObj = calloc(1, sizeof(DataObj)); 
	if(newObj == NULL){
		printf("Failed to create object of type '%s'.\n", class->name);
		return NULL;
	}
	if (parent == NULL) parent = &gameHeader;
	newObj->parent = parent;
	newObj->prev = NULL;
	newObj->next = parent->child;
	if (parent->child) {
		parent->child->prev = newObj;
	}
	parent->child = newObj;

	newObj->pos = (Vector3){0,0,0};
	newObj->transform = NULL;
	newObj->scale = (Vector3){1,1,1};
	newObj->rot = (Vector3){0,0,0};
	newObj->colour = (CharColour){255, 255, 255, 255};
	newObj->name = class->name;
	newObj->class = class;
	newObj->values = NULL;

	printf("Created new object of type '%s'.\n", class->name);
	
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
	CollsionReturn *output = NULL;
	
	if(itemA->shape == COLLHULL_CUBE && itemB->shape == COLLHULL_CUBE){
		if(!(between(itemA->pos.x, itemB->pos.x, itemB->pos.x + itemB->scale.x) && between(itemA->pos.y, itemB->pos.y, itemB->pos.y + itemB->scale.y) && between(itemA->pos.z, itemB->pos.z, itemB->pos.z + itemB->scale.z))) return NULL;
		output = malloc(sizeof(CollsionReturn));
		output->outNorm = (Vector3){0, itemA->pos.y - itemB->pos.y, 0};
	}
	
	/*if(collide is yes) then do'eth
		tell me the collision outputs then please
	  else*/
	return output;
	//end
}

//--------------------------------------------- temporarys until i put them into their own little file johnson

extern float timer;
extern double deltaTime;
extern Camera currentCamera;
extern KeyMap keyList[KEYBINDCOUNT];

extern Mesh *teapotMesh;
extern Mesh *playerMesh;
extern Mesh *cubeMesh;

extern SDL_Texture *playerTex;
extern SDL_Texture *homerTex;

void playerUpdate(DataObj* object){
	Vector3 oldPos = object->pos;
	object->pos.x += ((SDL_cos(currentCamera.rot.y) * (keyList[KEYBIND_D].down - keyList[KEYBIND_A].down)) + (SDL_sin(currentCamera.rot.y) * (keyList[KEYBIND_S].down - keyList[KEYBIND_W].down))) * 4 * deltaTime;
	object->pos.y += (keyList[KEYBIND_SPACE].down - keyList[KEYBIND_SHIFT].down) * 4 * deltaTime;
	object->pos.z += ((-SDL_sin(currentCamera.rot.y) * (keyList[KEYBIND_D].down - keyList[KEYBIND_A].down)) + (SDL_cos(currentCamera.rot.y) * (keyList[KEYBIND_S].down - keyList[KEYBIND_W].down))) * 4 * deltaTime;
	
	if(keyList[KEYBIND_W].down || keyList[KEYBIND_A].down || keyList[KEYBIND_S].down || keyList[KEYBIND_D].down){
		object->rot.y = atan2(oldPos.y - object->pos.y, oldPos.x - object->pos.x);
	}

	//object->pos.y = SDL_cos(timer) / 2 + 2;
	currentCamera.pos = (Vector3){object->pos.x + (SDL_cos(currentCamera.rot.x) * SDL_sin(currentCamera.rot.y)) * currentCamera.focusDist, object->pos.y + 2 - SDL_sin(currentCamera.rot.x) * currentCamera.focusDist, object->pos.z + (SDL_cos(currentCamera.rot.x) * SDL_cos(currentCamera.rot.y)) * currentCamera.focusDist};
}

void playerDraw(DataObj* object){
	drawMesh(playerMesh, object->transform, (SDL_FColor){1, 1, 1, 1});
	//drawCube((Vector3){object->pos.x - 1, object->pos.y + 4, object->pos.z - 1}, (Vector3){2, 4, 2}, (SDL_FColor){1, 1, 1, 1});
	//drawBillboard(playerTex, (SDL_FRect){0, 0, 128, 128}, object->pos, (SDL_FPoint){8, 16}, (SDL_FPoint){4, 4});
}

void blockDraw(DataObj* object){
	//drawCube(object->pos, object->scale, charColConv(object->colour));
	drawMesh(teapotMesh, object->transform, charColConv(object->colour));

	if (!strcmp(object->name, "RedBlock")) {
		Vector3 scaleNew = (Vector3){2 + SDL_cos(timer), SDL_sin(timer) + 1, 2 + SDL_cos(timer)};
		memcpy(&object->scale, &scaleNew, sizeof(float)*3);
	}
}

void homerDraw(DataObj* object){
	//drawCube(object->pos, object->scale, charColConv(object->colour));
	drawBillboard(homerTex, (SDL_FRect){0, 0, 300, 500}, object->pos, (SDL_FPoint){1.5, 2.5}, (SDL_FPoint){3, 5});
}

DataType playerClass = {"Player\0", 0, NULL, playerUpdate, playerDraw};
DataType fuckingBeerdrinkerClass = {"beer drinker\0", 0, NULL, NULL, homerDraw};
DataType blockClass = {"Block\0", 0, NULL, NULL, blockDraw};