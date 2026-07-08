#ifndef INSTANCES_H
#define INSTANCES_H

#include <SDL3/SDL.h>

#include "structs.h"

typedef struct NotiPopup{
	char* text;
	SDL_Texture* image;
	SDL_FRect* imageSrc;
	float age, life; //in seconds
	struct NotiPopup* prev;
	struct NotiPopup* next;
} NotiPopup;

typedef enum consoleLogTypes{
	CONSOLELOG_DEFAULT,
	CONSOLELOG_WARNING, CONSOLELOG_ERROR,
	CONSOLELOG_SERVER, CONSOLELOG_EXTRA, CONSOLELOG_MAX
} consoleLogTypes;

typedef struct ConsoleLog{
	char* text;
	Uint32 type;
	Uint32 count;

	struct ConsoleLog* prev;
	struct ConsoleLog* next;
} ConsoleLog;

typedef struct ScriptItem{
	void (*func)(DataObj*);
	char* funcName;
} ScriptItem;

DataObj* newObject(DataType* classData);
void removeObject(DataObj* object);
bool parentObject(DataObj* child, DataObj* parent);
DataObj* duplicateObject(DataObj* ogItem);

void updateObjects(DataObj* item);
void drawObjects(DataObj* item);
void cleanupObjects(DataObj* item);
void lesserCleanupObjects(DataObj* item);

DataObj* firstChildOfType(DataObj* item, DataType* classData);
DataObj* firstChildWithName(DataObj* item, char* name);
DataObj** listChildren(DataObj* item);

void sendPopup(char* string, SDL_Texture* image, SDL_FRect* rect, float life);
void renderPopups();
void updatePopups();

void logToConsole(char* string, Uint32 type);
void clearConsole();

#endif