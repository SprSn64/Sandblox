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

typedef struct StudioSplit{
	bool splitA, splitB; //true for another pair of panels
	void* childA;
	void* childB;

	float split; //weight of scale ratio between childA and childB, 0.5 for right in the center
	bool vert; //if the split is verticle (childA above childB)
} StudioSplit;

typedef enum panelTypes{
	PANEL_GAME, PANEL_EXPLORER, //explorer as in the list of objects
	PANEL_PROPERTIES, PANEL_CONSOLE,
	PANEL_TOOLBAR,
} panelTypes;

typedef struct StudioPanel{
	Uint32 type;
	//add something for extra tabs on a single panel
} StudioPanel;

HistoryItem* addHistoryItem(Uint32 type, void** items);
bool undoHistory(HistoryItem* item);

#endif