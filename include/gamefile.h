#ifndef GAMEFILE_H
#define GAMEFILE_H

#include <structs.h>

int loadGameFile(const char* filename);
int saveGameFile(const char* filename);
DataObj* createPlayerFromJSON();

#endif