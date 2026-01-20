#ifndef DEVSTUDIO_MAIN_H
#define DEVSTUDIO_MAIN_H

void initStudio();
void updateStudio();
void studioCleanup();

typedef enum studioToolMode{
	STUDIOTOOL_NONE,
	STUDIOTOOL_MOVE,
	STUDIOTOOL_SCALE,
	STUDIOTOOL_ROTATE,
} studioToolMode;

#endif