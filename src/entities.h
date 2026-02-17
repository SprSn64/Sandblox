#ifndef ENTITIES_H
#define ENTITIES_H

#include <structs.h>

/*void playerInit(DataObj* object);
void playerUpdate(DataObj* object);
void playerDraw(DataObj* object);
void blockDraw(DataObj* object);
void homerDraw(DataObj* object);

void scriptUpdate(dataObj* object);*/

extern DataType playerClass;
extern DataType blockClass;
extern DataType meshClass;
extern DataType groupClass;
extern DataType cameraClass;
//extern DataType lightClass;
extern DataType imageClass;
extern DataType scriptClass;
extern DataType accessoryClass;

extern DataType fuckingBeerdrinkerClass;

void objSpinFunc(DataObj* object);
void killBrickFunc(DataObj* object);

#endif