#ifndef DEVSTUDIO_PANELS_H
#define DEVSTUDIO_PANELS_H

#include <SDL3/SDL.h>
#include <structs.h>

void drawSplit(StudioSplit* item, SDL_FRect* area);
void updateSplit(StudioSplit* item, SDL_FRect* area);

#endif