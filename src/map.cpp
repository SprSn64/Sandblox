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
extern ClientData client;
}

std::map<std::string, DataType*> nameToClass = {
    { "blockClass", &blockClass },
};

std::string getDirFromFile(std::string path) {
    return std::filesystem::path(path).parent_path().string();
}

LINK_WITH_C void loadMapFromSBMap(const char *path) {
	char fullPath[512];
	sprintf(fullPath, "%s%s", SDL_GetCurrentDirectory(), path);
    FILE *f = fopen(fullPath, "r");
    if (!f) return;

    char line[512];
    while (fgets(line, sizeof(line), f)) {
        if (strncmp(line, "mapmesh", 7) == 0) {
            char mapmeshFile[64];
            sscanf(line, "mapmesh %s", mapmeshFile);
            std::string pathToDir = getDirFromFile(std::string(path));
            std::string pathToObj = pathToDir + "/" + mapmeshFile;

            Mesh *levelmesh = loadMeshFromObj(pathToObj.c_str());
            client.gameWorld->headObj->objMesh = levelmesh;
        } else if (strncmp(line, "startpos", 8) == 0) {
            float x,y,z;
            sscanf(line, "startpos %f, %f, %f", &x,&y,&z);
            playerObj = newObject(NULL, &playerClass);
            playerObj->pos.x = x;
            playerObj->pos.y = y;
            playerObj->pos.z = z;
        } else if (strncmp(line, "object", 6) == 0) {
            float x,y,z, rx,ry,rz;
		Uint8 r,g,b;
            char className[64];
            sscanf(line, "object %f, %f, %f, %f, %f, %f, %hhu, %hhu, %hhu, %s", &x,&y,&z, &rx,&ry,&rz, &r,&g,&b, className);
            DataType *type = nameToClass[className];
            DataObj *newObj = newObject(NULL, type);
            newObj->pos = (Vector3){x, y, z};
		newObj->scale = (Vector3){rx, ry, rz};
		newObj->colour = (CharColour){r, g, b, 255, 0, COLOURMODE_RGB};
        }
    }
    fclose(f);
}