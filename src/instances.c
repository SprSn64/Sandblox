#include <SDL3/SDL.h>
#include <SDL3_image/SDL_image.h>

#include <stdio.h>
#include <string.h>
#include <math.h>
#include <stdlib.h>

#include <structs.h>
#include "instances.h"
#include "renderer.h"
#include "math.h"
#include "loader.h"
#include "entities.h"
#include "physics.h"

extern SDL_Renderer *renderer;
extern SDL_Texture *fontTex;

extern ClientData client;
extern GameWorld game;
extern DataObj *focusObject;
extern float timer;

extern Uint32 objListLength;

DataObj gameHeader = {
	.pos = (Vector3){0, 0, 0},
	.scale = (Vector3){1, 1, 1},
	.rot = (Vector3){0, 0, 0},
	.transform = NULL,
	.colour = (CharColour){0, 0, 0, 255, 0, COLOURMODE_RGB},
	.name = "Workspace",
	.classData = &(DataType){
		"Workspace",
		1,
		0,
		NULL,
		NULL,
		NULL
	},
	.asVoidptr = {NULL},
	.asVec3 = {{0}},
	.asInt = {0},
	.asFloat = {0},
	.prev = NULL, .next = NULL, .parent = NULL, .child = NULL,
	.studioOpen = true,
};

extern Mesh *cubePrim;
void updateObjects(DataObj* item, int nodeDepth, int *idCount, bool uord){ //uord = update or draw
	//int i = (*idCount)++;
	if (!uord){
		item->rot = (Vector3){fmod(item->rot.x, 6.28318), fmod(item->rot.y, 6.28318), fmod(item->rot.z, 6.28318)};
		if(item->classData->update)item->classData->update(item);
		if(item->asVoidptr[OBJVAL_SCRIPT]){
			//sendPopup("fuck", NULL, NULL, 0.2);
			//item->asVoidptr[OBJVAL_SCRIPT](item); //doesnt work
		}
	}else{ 
		if(item->parent && item->parent->studioOpen == true)
			objListLength += item->parent->studioOpen;
		if(!item->classData->draw) goto noDraw;
		/*item->transform = newMatrix();
		scaleMatrix2(item->transform, item->scale);
		rotateMatrix2(item->transform, item->rot);
		translateMatrix2(item->transform, item->pos);*/
		
		item->transform = genMatrix(item->pos, item->scale, item->rot);
		
		item->classData->draw(item);
		if(item == focusObject && client.studio)drawMesh(cubePrim, item->transform, (SDL_FColor){1, 1, 1, fabs(SDL_sin(timer * 2)) * 0.25}, NULL, false);
		free(item->transform);
	}
	noDraw:
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
		sendPopup("are you out of your mind?", NULL, NULL, 5);
		return;
	}
	
	DataObj *prevItem = object->prev; 
	DataObj *nextItem = object->next; 
	DataObj *parentItem = object->parent ? object->parent : game.headObj;
	DataObj *childItem = object->child;
	
	if(prevItem)
		prevItem->next = nextItem;
	if(nextItem)
		nextItem->prev = prevItem;
	
	if(parentItem && parentItem->child == object)
		parentItem->child = nextItem;
	
	if(childItem){
		DataObj *loopItem = childItem;
		while(loopItem){
			loopItem->parent = parentItem;
			if(!loopItem->next) break;
			loopItem = loopItem->next;
		}
		
		if(parentItem->child == NULL){
			parentItem->child = childItem;
		} else {
			DataObj *lastChild = parentItem->child;
			while(lastChild->next)
				lastChild = lastChild->next;
			lastChild->next = childItem;
			childItem->prev = lastChild;
		}
	}
	
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
		if(loopItem->classData->id == classData.id)
			return loopItem;
		loopItem = loopItem->next;
	}
	return NULL;
}

DataObj** listChildren(DataObj* item){
	if(!item->child) return NULL;
	Uint32 childCount = 0;
	
	DataObj *loopItem = item->child;
	while(loopItem){
		childCount++;
		loopItem = loopItem->next;
	}
	
	DataObj **childList = malloc(sizeof(void) * childCount);
	loopItem = item->child;
	for(Uint32 i = 0; i < childCount && loopItem; i++){
		childList[i] = loopItem;
		loopItem = loopItem->next;
	}
	
	return childList;
}

//not dealing with a descendant list function yet

NotiPopup* popupHead = NULL;

void sendPopup(char* string, SDL_Texture* image, SDL_FRect* rect, float life){
	NotiPopup* newPopup = malloc(sizeof(NotiPopup));
	if(!newPopup) return;
	
	//printf("%s\n", string);
	
	newPopup->text = string;
	newPopup->image = image; newPopup->imageSrc = rect;
	newPopup->age = 0; newPopup->life = life;
	
	newPopup->last = NULL;
	if(popupHead)
		popupHead->last = newPopup;
	newPopup->next = popupHead;
	popupHead = newPopup;
}

extern double deltaTime;

void updatePopup(NotiPopup* item){
	item->age += deltaTime;
	if(item->age >= item->life){
		if(popupHead == item) popupHead = NULL;
		if(item->last) item->last->next = NULL;
		free(item);
	}
}

extern SDL_Point windowScale;
extern Font defaultFont;

void renderPopup(NotiPopup* item, Uint32 posX, Uint32 posY){
	if(item->age >= item->life) return;
	
	SDL_SetRenderDrawColor(renderer, 0, 0, 0, 128);
	SDL_RenderFillRect(renderer, &(SDL_FRect){posX, posY, 224, 64});
	
	drawText(renderer, &defaultFont, item->text, posX + 2, posY + 2, 1, (SDL_FColor){1, 1, 1, 1});
}

void updatePopups(){
	Uint32 popupCount = 0;
	NotiPopup* loopItem = popupHead;
	while(loopItem){
		popupCount++;
		renderPopup(loopItem, windowScale.x - 224, windowScale.y - (popupCount + 1) * 66);
		updatePopup(loopItem);
		
		loopItem = loopItem->next;
	}
}