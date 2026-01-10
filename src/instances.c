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

extern GameWorld game;
extern float timer;

void mapDraw(DataObj* object){
	drawMesh(object->objMesh, object->transform, (SDL_FColor){1, 1, 1, 1});
}

DataObj gameHeader = {
	(Vector3){0, 0, 0},
	(Vector3){1, 1, 1},
	(Vector3){0, 0, 0},
	NULL,
	(CharColour){0, 0, 0, 255, 0, COLOURMODE_RGB},
	"Workspace",
	&(DataType){
		"Workspace",
		1,
		0,
		NULL,
		NULL,
		mapDraw
	},
	NULL, NULL, NULL, NULL, NULL, NULL,
	true,
};

#define OBJLIST_HUD_POS_X 0
#define OBJLIST_HUD_POS_Y 32

void updateObjects(DataObj* item, int nodeDepth, int *idCount, bool uord){ //uord = update or draw
	//int i = (*idCount)++;
	item->transform = newMatrix();
	rotateMatrix2(item->transform, item->rot);
	translateMatrix2(item->transform, (Vector3){item->pos.x, item->pos.y, item->pos.z});
	scaleMatrix2(item->transform, (Vector3){item->scale.x, item->scale.y, item->scale.z});
	if (item->classData) {
		if (item->classData->update && !uord) item->classData->update(item);
		if (item->classData->draw && uord) item->classData->draw(item);
	}
	free(item->transform);
	//if(uord)drawText(renderer, fontTex, item->name, 32, OBJLIST_HUD_POS_X + (nodeDepth * 24), OBJLIST_HUD_POS_Y + i * 16, 16, 16, 12);
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

DataObj* newObject(DataObj* parent, DataType* classData){
	DataObj *newObj = calloc(1, sizeof(DataObj)); 
	if(newObj == NULL){
		printf("Failed to create object of type '%s'.\n", classData->name);
		return NULL;
	}
	newObj->parent = parent;
	if (parent == NULL) parent = game.headObj;
	
	/* first added is first rendered/updated, last added is last rendered/updated
	newObj->next = NULL;
	//newObj->next = parent->child;
	if(!parent->child){
		parent->child = newObj;
	}else{
		DataObj *loopItem = parent->child;
		while(loopItem->next){
			loopItem = loopItem->next;
		}
		loopItem->next = newObj;
		newObj->prev = loopItem;
	}
	*/
	
	// first added is last rendered/updated, last added is first rendered/updated
	newObj->prev = NULL;
	newObj->next = parent->child;
	if (parent->child)
		parent->child->prev = newObj;
	parent->child = newObj;

	newObj->pos = (Vector3){0,0,0};
	newObj->transform = NULL;
	newObj->scale = (Vector3){1,1,1};
	newObj->rot = (Vector3){0,0,0};
	newObj->colour = (CharColour){255, 255, 255, 255, 0, COLOURMODE_RGB};
	newObj->name = classData->name;
	newObj->classData = classData;
	
	newObj->studioOpen = false;
	//newObj->values = NULL;
	if (classData->init)
		classData->init(newObj);

	printf("Created new object of type '%s'.\n", classData->name);
	
	return newObj;
}

void removeObject(DataObj* object){
	//object->onRemove(object);
	
	if(object == game.headObj){
		printf("are you out of your mind?\n");
		return;
	}
	
	DataObj *prevItem = object->prev; DataObj *nextItem = object->next; DataObj *parentItem = object->parent; DataObj *childItem = object->child;
	
	if(prevItem)
		prevItem->next = nextItem;
	
	if(childItem){
		childItem->parent = parentItem;
		
		DataObj *loopItem = object->child;
		while(loopItem->next){
			loopItem->parent = parentItem;
			loopItem = loopItem->next;
		}
		
		loopItem = parentItem->child;
		while(loopItem->next){
			loopItem = loopItem->next;
		}
		loopItem->next = childItem;
	}
	
	//segmentation fault... wtf? getting data from gameHeader headObj causes error????
	
	if(parentItem->child == object)
		parentItem->child = nextItem;
	
	printf("Object '%s' removed.\n", object->name);
	free(object);
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

DataObj* firstChildOfType(DataObj* item, DataType classData){
	if(!item->child) return NULL;
	DataObj *loopItem = item->child;
	while(loopItem){
		if(loopItem->classData->id == classData.id){
			return loopItem;
		}
		loopItem = loopItem->next;
	}
	return NULL;
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

extern double deltaTime;
//extern Camera currentCamera;
extern ButtonMap keyList[KEYBINDCOUNT];

extern Mesh *teapotMesh;
extern Mesh *playerMesh;
extern Mesh *cubeMesh;
extern Mesh *cubePrim;
extern Mesh *spherePrim;

extern SDL_Texture *playerTex;
extern SDL_Texture *homerTex;

void playerInit(DataObj* object){
	object->pos.y = 0;
}

void playerUpdate(DataObj* object){
	if(object != game.currPlayer) return;
	
	Vector3 *playerVel = &object->objVel;
	
	SDL_FPoint playerMove = {0, 0};
	
	bool plrMoving = abs(keyList[KEYBIND_D].down - keyList[KEYBIND_A].down) + abs(keyList[KEYBIND_S].down - keyList[KEYBIND_W].down);
	
	if(plrMoving){
		playerMove = normalize2((SDL_FPoint){
			(SDL_cos(game.currCamera->rot.y) * (keyList[KEYBIND_D].down - keyList[KEYBIND_A].down)) + (SDL_sin(game.currCamera->rot.y) * (keyList[KEYBIND_S].down - keyList[KEYBIND_W].down)),
			(-SDL_sin(game.currCamera->rot.y) * (keyList[KEYBIND_D].down - keyList[KEYBIND_A].down)) + (SDL_cos(game.currCamera->rot.y) * (keyList[KEYBIND_S].down - keyList[KEYBIND_W].down)),
		});
		object->rot.y = atan2(playerMove.x, playerMove.y) + 3.14159;
	}
	
	if(game.currCamera->focusDist == 0){
		object->rot.y = game.currCamera->rot.y;
	}
	
	playerVel->x = (playerVel->x + playerMove.x) * 0.92;
	playerVel->z = (playerVel->z + playerMove.y) * 0.92;
	playerVel->y += -1 + 0.4 * (keyList[KEYBIND_SPACE].down && playerVel->y > 0);
	
	//object->pos.x += playerMove.x * 4 * deltaTime;
	//object->pos.y += (keyList[KEYBIND_SPACE].down - keyList[KEYBIND_SHIFT].down) * 4 * deltaTime;
	//object->pos.z += playerMove.y * 4 * deltaTime;
	object->pos = (Vector3){object->pos.x + playerVel->x * deltaTime, object->pos.y + playerVel->y * deltaTime, object->pos.z + playerVel->z * deltaTime};
	
	if(object->pos.y < 0){
		object->pos.y = 0;
		playerVel->y = 18 * keyList[KEYBIND_SPACE].pressed;
	}

	//object->pos.y = SDL_cos(timer) / 2 + 2;
	game.currCamera->pos = (Vector3){object->pos.x + (SDL_cos(game.currCamera->rot.x) * SDL_sin(game.currCamera->rot.y)) * game.currCamera->focusDist, object->pos.y + 2 - SDL_sin(game.currCamera->rot.x) * game.currCamera->focusDist, object->pos.z + (SDL_cos(game.currCamera->rot.x) * SDL_cos(game.currCamera->rot.y)) * game.currCamera->focusDist};
}


void playerDraw(DataObj* object){
	drawMesh(playerMesh, object->transform, (SDL_FColor){1, 1, 1, 1});
	//drawCube((Vector3){object->pos.x - 1, object->pos.y + 4, object->pos.z - 1}, (Vector3){2, 4, 2}, (SDL_FColor){1, 1, 1, 1});
	//drawBillboard(playerTex, (SDL_FRect){0, 0, 128, 128}, object->pos, (SDL_FPoint){8, 16}, (SDL_FPoint){4, 4});
}

DataType meshClass = (DataType){"Mesh\0", 4, 0, NULL, NULL, NULL};

void blockDraw(DataObj* object){
	//drawCube(object->pos, object->scale, ConvertSDLColour(object->colour));
	Mesh *itemMesh = cubePrim;
	DataObj *meshItem = firstChildOfType(object, meshClass);
	if(meshItem)
		itemMesh = meshItem->asVoidptr[OBJVAL_MESH];
	drawMesh(itemMesh, object->transform, ConvertSDLColour(object->colour));

	if (!strcmp(object->name, "Red Teapot")) {
		object->rot = (Vector3){object->rot.x + 0.02, object->rot.y + 0.02, object->rot.z};
	}
}

void homerDraw(DataObj* object){
	//drawCube(object->pos, object->scale, ConvertSDLColour(object->colour));
	drawBillboard(homerTex, (SDL_FRect){0, 0, 300, 500}, object->pos, (SDL_FPoint){1.5, 2.5}, (SDL_FPoint){3, 5});
}

DataType playerClass = {"Player\0", 2, 0, playerInit, playerUpdate, playerDraw};
DataType fuckingBeerdrinkerClass = {"beer drinker\0", 666, 0, NULL, NULL, homerDraw};
DataType blockClass = {"Block\0", 3, 0, NULL, NULL, blockDraw};