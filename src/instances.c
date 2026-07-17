#include <SDL3/SDL.h>
#include <SDL3_image/SDL_image.h>
#include <GL/glew.h>

#include <stdio.h>
#include <string.h>
#include <math.h>
#include <stdlib.h>

#include "softwarerender/main.h"

#include "structs.h"
#include "instances.h"
#include "renderer.h"
#include "math.h"
#include "mesh.h"
#include "entities.h"
#include "physics.h"
#include "opengl.h"

extern SDL_Renderer *renderer;
extern SDL_Texture *fontTex;

extern ClientData client;
extern GameWorld game;
extern DataObj *focusObject;
extern float timer;

extern Uint32 objListLength;

DataType workspaceClass = {"Workspace", 1, 0, NULL, NULL, NULL, NULL};

void logToConsole(char* string, Uint32 type);

DataObj gameHeader = {
	.pos = (Vector3){0, 0, 0},
	.scale = (Vector3){1, 1, 1},
	.rot = (Vector3){0, 0, 0},
	.transform = NULL,
	.colour = (CharColour){0, 0, 0, 255, 0, COLOURMODE_RGB},
	.name = "Workspace",
	.classData = &workspaceClass,
	.props = {NULL},
	.prev = NULL, .next = NULL, .parent = NULL, .child = NULL,
	.studioOpen = true,
};

void updateObjects(DataObj* item){
	item->rot = (Vector3){fmod(item->rot.x, PI * 2), fmod(item->rot.y, PI * 2), fmod(item->rot.z, PI * 2)};
	if(item->classData->update)item->classData->update(item);
	CollisionHull* collision = item->objColl;
	if(collision)collision->pos = vec3Add(item->pos, collision->offset);

	DataObj* child = item->child;
	while (child) {
		DataObj *next = child->next;
		updateObjects(child);
		child = next;
	}
}

extern Mesh *cubePrim;
extern Mesh *spherePrim;
void drawObjects(DataObj* item){
	if(!item->classData->draw) goto noDraw;

	CollisionHull *collider = item->objColl;
	if(collider && (collider->shape == COLLHULL_CUBE || collider->shape == COLLHULL_SPHERE)){
		Mesh* collMesh = cubePrim;
		switch(collider->shape){
			case COLLHULL_SPHERE: collMesh = spherePrim; break;
		}

		glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
		float* collTransform = genMatrix(collider->pos, collider->scale, (Vector3){0, 0, 0});
		drawMeshOpenGL(collMesh, collTransform, (SDL_FColor){1, 0, 0, 1}, NULL);
		free(collTransform);
		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	}
	
	item->transform = genMatrix(item->pos, item->scale, item->rot);
	item->classData->draw(item);
	if(item == focusObject && client.studio){
		setGlValue(GL_DEPTH_TEST, false);
		drawMeshOpenGL(cubePrim, item->transform, (SDL_FColor){1, 1, 1, fabs(SDL_sin(timer * 2)) * 0.25}, NULL);
		setGlValue(GL_DEPTH_TEST, true);
	}
	free(item->transform);

noDraw:
	DataObj* child = item->child;
	while (child) {
		DataObj *next = child->next;
		drawObjects(child);
		child = next;
	}
}

DataObj* newObject(DataType* classData){
	DataObj *newObj = calloc(1, sizeof(DataObj)); 
	if(newObj == NULL){
		char* errorLog = malloc(256);
		sprintf(errorLog, "Failed to create object of type '%s'.\n", classData->name);
		logToConsole(errorLog, CONSOLELOG_ERROR);
		return NULL;
	}
	/*newObj->parent = parent;
	if (parent == NULL) parent = game.headObj;
	
	// first added is first rendered/updated, last added is last rendered/updated
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
	}*/
	
	
	// first added is last rendered/updated, last added is first rendered/updated
	/*
	newObj->prev = NULL;
	newObj->next = parent->child;
	if (parent->child)
		parent->child->prev = newObj;
	parent->child = newObj;
	*/

	for (int prop = 0; prop < OBJVAL_MAX; prop++) {
		newObj->props[prop] = NULL;
	}

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

	char* successLog = malloc(256);
	sprintf(successLog, "Created new object of type '%s'.\n", classData->name);
	logToConsole(successLog, CONSOLELOG_EXTRA);
	
	return newObj;
}

DataObj* duplicateObject(DataObj* ogItem){
	DataObj* newItem = newObject(ogItem->classData);
	parentObject(newItem, ogItem->parent);
	newItem->pos = ogItem->pos; newItem->scale = ogItem->scale; newItem->rot = ogItem->rot; newItem->colour = ogItem->colour;
	char* newName = malloc(256);
	sprintf(newName, "%s (Copy)", ogItem->name);
	newItem->name = newName;
	
	newItem->props[OBJVAL_SCRIPT] = ogItem->props[OBJVAL_SCRIPT];
	if(ogItem->objColl){
		CollisionHull* newColl = malloc(sizeof(CollisionHull)); memcpy(newColl, ogItem->objColl, sizeof(CollisionHull));
		newItem->objColl = newColl;
	}
	
	return newItem;
}

void removeObject(DataObj* object){
	if (object->classData->destroy)
		object->classData->destroy(object);
	
	if(object == game.headObj){
		sendPopup("are you out of your mind?", NULL, NULL, 5);
		return;
	}
	
	DataObj *prevItem = object->prev; 
	DataObj *nextItem = object->next; 
	DataObj *parentItem = object->parent ? object->parent : game.headObj;
	DataObj *childItem = object->child;

	if(focusObject == object)
		focusObject = NULL;
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
		
		if(parentItem->child == NULL) parentItem->child = childItem;
		else {
			DataObj *lastChild = parentItem->child;
			while(lastChild->next) lastChild = lastChild->next;
			lastChild->next = childItem;
			childItem->prev = lastChild;
		}
	}
	
	//printf("Object '%s' removed.\n", object->name);
	/*for (int prop = 0; prop < OBJVAL_MAX; prop++) {
		void *ptr = object->props[prop];
		if (ptr) free(ptr);
	}*/
	free(object);
}

void cleanupObjects(DataObj* item){
	DataObj* child = item->child;
	while (child) {
		DataObj *next = child->next;
		cleanupObjects(child);
		child = next;
	}
	if (&gameHeader != item) removeObject(item);
}

void lesserCleanupObjects(DataObj* item){
	DataObj* child = item->child;
	while (child) {
		DataObj *next = child->next;
		cleanupObjects(child);
		removeObject(item);
		child = next;
	}
}

bool parentObject(DataObj* child, DataObj* parent){
	if(child == parent || parent->parent == child) return 0;

	child->parent = parent;
	child->next = NULL;

	if(!parent->child){
		parent->child = child;
	}else{
		DataObj *loopItem = parent->child;
		while(loopItem->next){
			loopItem = loopItem->next;
		}
		loopItem->next = child;
		child->prev = loopItem;
	}

	//printf("%s -> %s\n", parent->name, loopItem->next->name);

	return 0;
}

DataObj* firstChildOfType(DataObj* item, DataType* classData){
	if(!item->child) return NULL;
	DataObj *loopItem = item->child;
	while(loopItem){
		if(loopItem->classData->id == classData->id)
			return loopItem;
		loopItem = loopItem->next;
	}
	//printf("Couldn't find child of %s with class '%s'.\n", item->name, classData->name);
	return NULL;
}

DataObj* firstChildWithName(DataObj* item, char* name){
	if(!item->child) return NULL;
	DataObj *loopItem = item->child;
	while(loopItem){
		if(!strcmp(loopItem->name, name))
			return loopItem;
		loopItem = loopItem->next;
	}
	//printf("Couldn't find child of %s with name '%s'.\n", item->name, name);
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

extern float* defaultMatrix;
void transformObject(DataObj* item, Vector3 pos, Vector3 scale, Vector3 rot){
	float* rotMatrix = rotateMatrix(defaultMatrix, rot, ROT_XYZ);
	Vector3 rotPos = vec4ToVec3(matrixMult(vec3ToVec4(item->pos), rotMatrix));
	item->pos = (Vector3){rotPos.x * scale.x + pos.x, rotPos.y * scale.y + pos.y, rotPos.z * scale.z + pos.z};
	free(rotMatrix);

	item->scale = vec3Mult(item->scale, scale);
	item->rot = vec3Add(item->rot, rot);
}

NotiPopup* popupHead = NULL;

void sendPopup(char* string, SDL_Texture* image, SDL_FRect* rect, float life){
	NotiPopup* newPopup = malloc(sizeof(NotiPopup));
	if(!newPopup) return;
	
	//printf("%s\n", string);
	
	newPopup->text = string;
	newPopup->image = image; newPopup->imageSrc = rect;
	newPopup->age = 0; newPopup->life = life;
	
	newPopup->prev = NULL;
	if(popupHead)
		popupHead->prev = newPopup;
	newPopup->next = popupHead;
	popupHead = newPopup;
}

extern double deltaTime;

void updatePopup(NotiPopup* item){
	item->age += deltaTime;
	if(item->age >= item->life){
		if(popupHead == item) popupHead = NULL;
		if(item->prev) item->prev->next = NULL;
		free(item->text);
		free(item);
	}
}

extern SDL_Point windowScale;
extern Font defaultFont;

extern float aspectRatio;
extern Mesh* planePrim;
extern TextureRef* textBufferTex;
void renderPopup(NotiPopup* item, Uint32 posX, Uint32 posY){
	if(item->age >= item->life) return;
	
	SDL_SetRenderDrawColor(renderer, 0, 0, 0, 128);
	SDL_RenderFillRect(renderer, &(SDL_FRect){posX, posY, 224, 64});
	//drawRect(displayTex, posX, posY, 224, 64, 0x80000000);

	float* backMatrix = genMatrix(
		screenToGL((Vector3){posX, posY, 0}),
		(Vector3){(224.f / windowScale.x) * aspectRatio * 2, 1, (64.f / windowScale.y) * 2}, 
		(Vector3){HALFPI, 0, 0}
	);
	drawMeshOpenGL(planePrim, backMatrix, (SDL_FColor){0, 0, 0, 0.5}, NULL);
	free(backMatrix);
	
	//drawText(renderer, &defaultFont, item->text, posX + 2, posY + 2, 1, (SDL_FColor){1, 1, 1, 1});
	//drawRasterText(displayTex, &defaultFont, item->text, posX + 2, posY + 2, 1, 0xFFFFFFFF);

	float textScale = screenToGL((Vector3){8 + (windowScale.x >> 1), 1, 0}).x;
	drawGlText(&defaultFont, screenToGL((Vector3){posX + 2, posY + 2, 0}), item->text, textScale, (SDL_FColor){1, 1, 1, 1});
}

void updatePopups(){
	NotiPopup* loopItem = popupHead;
	while(loopItem){
		updatePopup(loopItem);
		
		loopItem = loopItem->next;
	}
}

void renderPopups(){
	Uint32 popupCount = 0;
	NotiPopup* loopItem = popupHead;
	while(loopItem){
		popupCount++;
		renderPopup(loopItem, windowScale.x - 224, windowScale.y - (popupCount + 1) * 66);
		
		loopItem = loopItem->next;
	}
}

ConsoleLog *consoleHead = NULL;
ConsoleLog *consoleLast = NULL;

extern int consoleScroll;

void logToConsole(char* string, Uint32 type){
	if(consoleLast && consoleLast->type == type && !strcmp(consoleLast->text, string)){
		consoleLast->count += 1;
		return;
	}

	ConsoleLog* newLog = malloc(sizeof(ConsoleLog));
	if(!newLog) return;
	
	newLog->text = string;
	newLog->type = type;
	newLog->count = 1;

	consoleScroll++;
	
	newLog->prev = consoleLast;
	newLog->next = NULL;

	/*if(consoleScroll >= 16 && consoleHead){
		consoleHead = consoleHead->next;
		consoleHead->next->prev = NULL;
		free(consoleHead);
	}*/

	if(!consoleHead){
		consoleHead = newLog;
		consoleLast = newLog;
		newLog->prev = NULL;
		return;
	}

	if(consoleLast){
		consoleLast->next = newLog;
	}

	consoleLast = newLog;
}

void clearConsole(){
	ConsoleLog *currLog = consoleHead;
	while(currLog){
		ConsoleLog* next = currLog->next;
		if(currLog->text)free(currLog->text);
		free(currLog);
		currLog = next;
	}

	consoleHead = NULL; consoleLast = NULL;
}