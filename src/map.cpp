#include <SDL3/SDL.h>

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <map>
#include <string>
#include <filesystem>

#include "map.h"

#define LINK_WITH_C extern "C"

LINK_WITH_C {
#include "instances.h"
#include "loader.h"

extern DataObj* playerObj;
extern DataType playerClass;
extern DataType blockClass;
extern DataObj gameHeader;
}

std::map<std::string, DataType*> nameToClass = {
    { "blockClass", &blockClass },
};

std::string getDirFromFile(std::string path) {
    return std::filesystem::path(path).parent_path().string();
}

LINK_WITH_C void loadMapFromSBMap(const char *path) {
    FILE *f = fopen(path, "r");
    if (!f) return;

    char line[512];
    while (fgets(line, sizeof(line), f)) {
        if (strncmp(line, "mapmesh", 7) == 0) {
            char mapmeshFile[64];
            sscanf(line, "mapmesh %s", mapmeshFile);
            std::string pathToDir = getDirFromFile(std::string(path));
            std::string pathToObj = pathToDir + "/" + mapmeshFile;

            Mesh *levelmesh = loadMeshFromObj(pathToObj.c_str());
            gameHeader.objMesh = levelmesh;
        } else if (strncmp(line, "startpos", 8) == 0) {
            float x,y,z;
            sscanf(line, "startpos %f, %f, %f", &x,&y,&z);
            playerObj = newObject(NULL, &playerClass);
            playerObj->pos.x = x;
            playerObj->pos.y = y;
            playerObj->pos.z = z;
        } else if (strncmp(line, "object", 6) == 0) {
            float x,y,z, rx,ry,rz;
            char className[64];
            sscanf(line, "object %f, %f, %f, %f, %f, %f, %s", &x,&y,&z, &rx,&ry,&rz, className);
            DataType *type = nameToClass[className];
            DataObj *newObj = newObject(NULL, type);
            newObj->pos.x = x;
            newObj->pos.y = y;
            newObj->pos.z = z;
        }
    }
    fclose(f);
}