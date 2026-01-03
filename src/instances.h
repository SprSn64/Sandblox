#ifndef INSTANCES_H
#define INSTANCES_H

#include <SDL3/SDL.h>

#include <structs.h>

DataObj* newObject(DataObj* parent, DataType* class);
void updateObjects(DataObj* item, int nodeDepth, int *idCount, bool uord);
void cleanupObjects(DataObj* item);
bool parentObject(DataObj* child, DataObj* parent);

#endif