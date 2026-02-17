#ifndef DEVSTUDIO_MAIN_H
#define DEVSTUDIO_MAIN_H

#include <SDL3/SDL.h>

void initStudio();
void updateStudio();
void studioCleanup();

typedef enum studioToolMode{
	STUDIOTOOL_NONE,
	STUDIOTOOL_MOVE,
	STUDIOTOOL_SCALE,
	STUDIOTOOL_ROTATE,
} studioToolMode;

typedef enum historyType{
	HISTORY_CHANGEVAL,
	HISTORY_ADDOBJ, HISTORY_DELOBJ,
	HISTORY_PARENTOBJ, HISTORY_MOVEOBJ, //HISTORY_MOVEOBJ means change order within child list
} historyType;

typedef struct HistoryItem{
	Uint32 type;
	void **items; //pointer to list
	struct HistoryItem* next;
	struct HistoryItem* prev;
} HistoryItem;

#endif