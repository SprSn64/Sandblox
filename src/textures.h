#ifndef TEXTURES_H
#define TEXTURES_H

#include "structs.h"
#include "renderer.h"

SDL_Texture *newTexture(char* path, SDL_ScaleMode scaleMode);
Texture* newRasterTexture(Uint16 width, Uint16 height);
Texture* loadRasterTexture(char* path);
bool freeRasterTexture(Texture* tex);

TextureRef* loadTexture(char* path, bool persistent);
void freeTexture(TextureRef* tex);
void updateGlTexture(TextureRef* tex);
void cleanupTextures(bool soft);

#endif // TEXTURES_H