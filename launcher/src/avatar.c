#include <structs.h>
#include "avatar.h"
#include "cjosn/cJSON.h"

#include <SDL3/SDL.h>
#include <SDL3_image/SDL_image.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

extern SDL_Window *window;
extern SDL_Renderer *renderer;

extern char* basePath;

SDL_Texture* avatarBaseTex = NULL;

extern Font defaultFont;
SDL_Texture *newTexture(char* path, SDL_ScaleMode scaleMode);
void drawText(SDL_Renderer *renderLoc, Font *textFont, char* text, short posX, short posY, float scale, SDL_FColor colour);

char* playerName;

void initAvatar(){
	avatarBaseTex = newTexture("assets/avatar/avatarbase.png", SDL_SCALEMODE_LINEAR);

	char *avatarPath = malloc(256); sprintf(avatarPath, "%splayer.json", basePath);
	FILE* file = fopen(avatarPath, "r");
	if(!file){
		printf("Failed load player file %s\n", avatarPath);
		goto avatarLoadSkip;
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
      	goto avatarLoadSkip;
    	}

    	cJSON* name = cJSON_GetObjectItem(json, "name");
    	if(name && cJSON_IsString(name))
        	playerName = strdup(name->valuestring);

      cJSON_Delete(json);
      free(content);

avatarLoadSkip:
	free(avatarPath);
}

void drawAvatar(SDL_Point pos){
	SDL_FRect sourceRect = (SDL_FRect){0, 0, 240, 320};
	SDL_FRect drawRect = (SDL_FRect){pos.x, pos.y, 240, 320};

	SDL_SetRenderDrawColor(renderer, 192, 192, 192, 255);
	SDL_RenderFillRect(renderer, &drawRect);

	SDL_RenderTexture(renderer, avatarBaseTex, &sourceRect, &drawRect);
	drawText(renderer, &defaultFont, playerName, pos.x, pos.y, 1, (SDL_FColor){0, 0, 0, 1});
}