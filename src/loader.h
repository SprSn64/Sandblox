#ifndef LOADER_H
#define LOADER_H

#include "structs.h"
#include "renderer.h"

Mesh *loadMeshFromObj(char* path);
char* loadTextFile(char* dir);

char* joinDirectories(char* dirA, char* dirB);
char* formatDirectory(char* dir);

#endif