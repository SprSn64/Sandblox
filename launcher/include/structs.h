#ifndef STRUCTS_H
#define STRUCTS_H

#include <SDL3/SDL.h>

#define max(a,b) ((a) > (b) ? (a) : (b))
#define min(a,b) ((a) < (b) ? (a) : (b))

typedef struct{
	bool down, pressed, released, pressCheck;
	Uint32 code;
} ButtonMap;

typedef struct{
	SDL_Texture *image;
	Uint16 startChar; //32 starts at the space glyth
	SDL_Point glyphSize;
	SDL_FPoint kerning;
	Uint16 columns;
} Font;

#endif