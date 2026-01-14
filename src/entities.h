#ifndef ENTITIES_H
#define ENTITIES_H

#include <structs.h>

void playerInit(DataObj* object);
void playerUpdate(DataObj* object);
void playerDraw(DataObj* object);
void blockDraw(DataObj* object);
void homerDraw(DataObj* object);

extern DataType playerClass;
extern DataType blockClass;
extern DataType meshClass;
extern DataType fuckingBeerdrinkerClass;

#endif