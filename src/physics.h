#ifndef PHYSICS_H
#define PHYSICS_H

#include "structs.h"

typedef enum MinDist{
	MINDIST_PX, MINDIST_NX, //P = positive, N = negative
	MINDIST_PY, MINDIST_NY,
	MINDIST_PZ, MINDIST_NZ,
} MinDist;

float checkBlockCollisionY(Vector3 pos, float footY, DataObj* block);
float findFloorY(Vector3 pos, float footY, DataObj* item);

Vector3 lazyCollisionLoop(DataObj* object, DataObj* item);

RayCast* castRay(Vector3 pos, Vector3 rot);
RayReturn* getRayIntersect(RayCast* ray, DataObj* item);
Vector3 rayDistPos(RayCast* ray, float dist);

#endif