#ifndef DEVSTUDIO_MAIN_H
#define DEVSTUDIO_MAIN_H

#include <SDL3/SDL.h>

void initStudio();
void updateStudio();
void freeStudio();

typedef enum studioToolMode{
	STUDIOTOOL_NONE,
	STUDIOTOOL_MOVE,
	STUDIOTOOL_SCALE,
	STUDIOTOOL_ROTATE,
} studioToolMode;

#endif