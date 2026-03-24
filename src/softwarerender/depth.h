#ifndef SOFTREND_DEPTH_H
#define SOFTREND_DEPTH_H

#include <SDL3/SDL.h>
#include <structs.h>
#include "main.h"

void drawDepthPixel(Texture* target, Uint16 x, Uint16 y, float z, Uint32 colour);
void drawDepthHamLine(Texture* target, Vector3 pointA, Vector3 pointB, Uint32 colour);
void drawDepthTriangle(Texture* target, Vector3 pointA, Vector3 pointB, Vector3 pointC, Uint32 colour);

#endif
