#ifndef SOFTREND_MAIN_H
#define SOFTREND_MAIN_H

#include <SDL3/SDL.h>
#include <structs.h>

#define WHITE 0xFFFFFFFF

typedef struct Texture{
	Uint32* pixels;
	Uint16 width, height;
} Texture;

Texture* newSoftwareTexture(Uint16 width, Uint16 height);
Texture* loadSoftwareTexture(char* path);

void setPixel(Texture* target, Uint16 x, Uint16 y, Uint32 colour);
Uint32 getPixel(Texture* target, Uint16 x, Uint16 y);
void clearTex(Texture* target, Uint32 colour);

void drawTexture(Texture* target, Texture* tex, SDL_Rect* source, SDL_Rect* dest, Uint32 colour);
//void drawText(Texture* target, Font *textFont, char* text, short posX, short posY, float scale, Uint32 colour);

void drawRect(Texture* target, Uint16 posX, Uint16 posY, Uint16 width, Uint16 height, Uint32 colour);
void drawHamLine(Texture* target, SDL_Point pointA, SDL_Point pointB, Uint32 colour);

#endif
