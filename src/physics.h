#ifndef PHYSICS_H
#define PHYSICS_H

#include <structs.h>

float checkBlockCollisionY(Vector3 pos, float footY, DataObj* block);
float findFloorY(Vector3 pos, float footY, DataObj* item);

#endif