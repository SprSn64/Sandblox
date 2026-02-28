#include "loader.h"
#include <structs.h>

#include <SDL3/SDL.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>

MapEntry* mapListHead = NULL;

MapEntry* addMapEntry(char* path, char* name){
	MapEntry* newEntry = malloc(sizeof(MapEntry));
	newEntry->path = malloc(256); sprintf(newEntry->path, "%s", path);
	newEntry->name = malloc(256); sprintf(newEntry->name, "%s", name);

	//child->parent = parent;
	newEntry->next = NULL;

	if(!mapListHead){
		mapListHead = newEntry;
		return newEntry;
	}

	MapEntry *loopItem = mapListHead;
	while(loopItem->next){
		loopItem = loopItem->next;
	}
	loopItem->next = newEntry;
	newEntry->prev = loopItem;

	return newEntry;
}

bool loadMapDir(char* path){
	struct dirent* entry;

	DIR *dir = opendir(path);
	if(!dir){
		printf("Couldn't open map directory (%s)\n", path);
		return 1;
	}

	char fullPath[256];
	while((entry = readdir(dir))){
		printf("%s\n", entry->d_name);
		sprintf(fullPath, "%s/%s", path, entry->d_name);

		char* extChar = strrchr(entry->d_name, '.');
		if(!extChar) continue;

		if(!strcmp(extChar, ".json"))
			addMapEntry(fullPath, entry->d_name);
      }

	closedir(dir); 
	return 0;
}