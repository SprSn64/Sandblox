#ifndef DEVSTUDIO_MAIN_H
#define DEVSTUDIO_MAIN_H

#include <SDL3/SDL.h>
#include "structs.h"

#include "../opengl.h"

typedef enum StudioToolMode{
	STUDIOTOOL_NONE,
	STUDIOTOOL_MOVE,
	STUDIOTOOL_SCALE,
	STUDIOTOOL_ROTATE,
} StudioToolMode;

typedef enum StudioPanelType{
	STUDIOPANEL_SPLIT,
	STUDIOPANEL_GAMEVIEW,
	STUDIOPANEL_TOOLBAR,
	STUDIOPANEL_TOOLS,
	STUDIOPANEL_EXPLORER,
	STUDIOPANEL_PROPERTIES,
	STUDIOPANEL_CONSOLE,
} StudioPanelType;

typedef struct StudioPanel{
	StudioPanelType type;
	void* handler;

	bool mouseOver;

	struct StudioPanel* parent;
} StudioPanel;

typedef struct SplitPanelHandler{
	StudioPanel* childA;
	StudioPanel* childB;
	float ratio; bool vert;
} SplitPanelHandler;

typedef struct GamePanelHandler{
	FrameBuffer* buffer;
	Camera* camera;
} GamePanelHandler;

void initStudio();
void updateStudio();
void renderStudio();
void freeStudio();

#endif