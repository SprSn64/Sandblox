#ifndef LOADER_H
#define LOADER_H

#include "structs.h"
#include "renderer.h"

SDL_Texture *newTexture(char* path, SDL_ScaleMode scaleMode);
Texture* newRasterTexture(Uint16 width, Uint16 height);
Texture* loadRasterTexture(char* path);
bool freeRasterTexture(Texture* tex);

TextureRef* loadTexture(char* path);
void freeTexture(TextureRef* tex);

Mesh *loadMeshFromObj(char* path);
char* loadTextFile(char* dir);

char* joinDirectories(char* dirA, char* dirB);
char* formatDirectory(char* dir);

#endif