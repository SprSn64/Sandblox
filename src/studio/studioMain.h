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

typedef struct StudioPanel{
	//something defining the type or info or something idk
	//add something else for multiple tabs on a single panel maybe
	struct StudioPanel* childA;
	struct StudioPanel* childB;

	float split; //weight of scale ratio between childA and childB, 0.5 for right in the center
	bool vert; //if the split is verticle (childA above childB)
} StudioPanel;

HistoryItem* addHistoryItem(Uint32 type, void** items);
bool undoHistory(HistoryItem* item);

#endif