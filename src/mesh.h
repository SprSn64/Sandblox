#ifndef LOADER_H
#define LOADER_H

#include "structs.h"
#include "renderer.h"
#include "textures.h"

Mesh *loadMeshFromObj(char* path, bool persistent);
Mesh* genTorusMesh(float outerRad, float innerRad, Uint16 ringRes, Uint16 ringCount);
Mesh* genCylinderMesh(float btmRad, float topRad, float length, int res);
Mesh* genPlaneMesh(float xScale, float yScale, Uint16 xRes, Uint16 yRes);
void cleanupMeshes(bool soft);

#endif