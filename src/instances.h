#ifndef INSTANCES_H
#define INSTANCES_H

#include <SDL3/SDL.h>

#include <structs.h>
#include <obj_fields.h>

DataObj* newObject(DataObj* parent, DataType* classData);
void removeObject(DataObj* object);
bool parentObject(DataObj* child, DataObj* parent);

void updateObjects(DataObj* item, int nodeDepth, int *idCount, bool uord);
void cleanupObjects(DataObj* item);

#endif