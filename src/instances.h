#ifndef INSTANCES_H
#define INSTANCES_H

#include <SDL3/SDL.h>

#include <structs.h>
#include <obj_fields.h>

typedef struct NotiPopup{
	char* text;
	SDL_Texture* image;
	SDL_FRect* imageSrc;
	float age, life; //in seconds
	struct NotiPopup* next;
	struct NotiPopup* last;
} NotiPopup;

typedef struct ScriptItem{
	void (*func)(DataObj*);
	char* funcName;
} ScriptItem;

DataObj* newObject(DataType* classData);
void removeObject(DataObj* object);
bool parentObject(DataObj* child, DataObj* parent);
DataObj* duplicateObject(DataObj* ogItem);

void updateObjects(DataObj* item, int nodeDepth, int *idCount, bool uord);
void cleanupObjects(DataObj* item);
void lesserCleanupObjects(DataObj* item);

DataObj* firstChildOfType(DataObj* item, DataType classData);
DataObj** listChildren(DataObj* item);

void sendPopup(char* string, SDL_Texture* image, SDL_FRect* rect, float life);
void updatePopups();

#endif