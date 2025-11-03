#ifndef RENDERER_H
#define RENDERER_H

#include <SDL3/SDL.h>

SDL_FPoint isoProj(Vector3 pos);
bool drawTriangle(SDL_FPoint pointA, SDL_FPoint pointB, SDL_FPoint pointC, SDL_FColor colour);
void drawCube(Vector3 pos, Vector3 scale, SDL_FColor colour);

#endif