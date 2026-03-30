#ifndef SOFTREND_DEPTH_H
#define SOFTREND_DEPTH_H

#include <SDL3/SDL.h>
#include <structs.h>
#include "main.h"
#include "../renderer.h"

Vector3 toScreen(Texture* target, Vector3 pos);

void drawDepthPixel(Texture* target, Uint16 x, Uint16 y, float z, Uint32 colour);
void drawDepthHamLine(Texture* target, Vector3 pointA, Vector3 pointB, Uint32 colourA, Uint32 colourB);
void drawDepthTriangle(Texture* target, MeshVert vertA, MeshVert vertB, MeshVert vertC, Texture* tex);

#endif
