#ifndef INSTANCES_H
#define INSTANCES_H

#include <SDL3/SDL.h>

#include <structs.h>

DataObj* newObject(DataObj* parent, DataType* class);
void updateObject(DataObj* item, int nodeDepth, int *idCount);

#endif