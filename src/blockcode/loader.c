#include "loader.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../cjosn/cJSON.h"
#include "structs.h"

extern char* basePath;

CodeBlockHeader* loadCodeFile(char* path){
	FILE* file = fopen(path, "r");
	if(!file){
		printf("Failed to find code file %s\n", path);
		return NULL;
	}

	fseek(file, 0, SEEK_END);
	long fileSize = ftell(file);
	fseek(file, 0, SEEK_SET);
	
	char* content = malloc(fileSize + 1);
	int readThing = fread(content, 1, fileSize, file); (void)readThing;
	content[fileSize] = '\0';
	fclose(file);

	cJSON* json = cJSON_Parse(content);
	if(!json){
		printf("Failed to parse JSON: %s\n", cJSON_GetErrorPtr());
		free(content);
		return NULL;
	}
	
	/*printf("Found %d objects in JSON\n", cJSON_GetArraySize(objects));
	
	int objectCount = cJSON_GetArraySize(objects);
	for(int i = 0; i < objectCount; i++) {
		cJSON* obj = cJSON_GetArrayItem(objects, i);
		if(!obj)continue;
	}*/

	free(content);
	return NULL;
}