#ifndef INSTANCES_H
#define INSTANCES_H

#include <SDL3/SDL.h>

#include <structs.h>

void drawObjList(int posX, int posY);
DataObj* newObject(DataType* class);
bool parentObject(DataObj* child, DataObj* parent);

Uint8 parentChildLoop(DataObj* currObj, Uint16* yIndex, Uint16* depth);

#endif