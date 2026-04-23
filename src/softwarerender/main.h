#ifndef SOFTREND_MAIN_H
#define SOFTREND_MAIN_H

#include <SDL3/SDL.h>
#include <structs.h>

#include "../renderer.h"

#define WHITE 0xFFFFFFFF

Uint32 colourToInt(SDL_FColor colour);
SDL_FColor intToColour(Uint32 colour);

Uint32 colourLerp(Uint32 colA, Uint32 colB, float t);

Texture* newRasterTexture(Uint16 width, Uint16 height);
Texture* loadRasterTexture(char* path);
bool freeRasterTexture(Texture* tex);

void setPixel(Texture* target, Uint16 x, Uint16 y, Uint32 colour);
Uint32 getPixel(Texture* target, Uint16 x, Uint16 y);
void clearTex(Texture* target, Uint32 colour);

void drawTexture(Texture* target, Texture* tex, SDL_Rect* source, SDL_Rect* dest, Uint32 colour);
void drawRasterText(Texture* target, Font *textFont, char* text, short posX, short posY, float scale, Uint32 colour);

void drawRect(Texture* target, Uint16 posX, Uint16 posY, Uint16 width, Uint16 height, Uint32 colour);
void drawHamLine(Texture* target, SDL_Point pointA, SDL_Point pointB, Uint32 colour);

#endif
