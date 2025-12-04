#ifndef INSTANCES_H
#define INSTANCES_H

#include <SDL3/SDL.h>

#include <structs.h>

void drawObjList(int posX, int posY);
DataObj* newObject(DataType* class);

#endif