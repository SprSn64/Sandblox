#ifndef LOADER_H
#define LOADER_H

#include "structs.h"
#include "renderer.h"

Mesh *loadMeshFromObj(const char* path);
char* loadTextFile(char* dir);

#endif