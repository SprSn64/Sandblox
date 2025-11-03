#ifndef RENDERER_H
#define RENDERER_H

#include <SDL3/SDL.h>
#include <SDL3_image/SDL_image.h>

SDL_Texture *newTexture(char* path);
void drawText(SDL_Renderer *renderer, SDL_Texture *texture, char* text, char charOff, short posX, short posY, short width, short height, short kern);

Vector3 worldToCamera(Vector3 pos);
SDL_FPoint isoProj(Vector3 pos);
bool drawTriangle(SDL_FPoint pointA, SDL_FPoint pointB, SDL_FPoint pointC, SDL_FColor colour);
void drawCube(Vector3 pos, Vector3 scale, SDL_FColor colour);

#endif