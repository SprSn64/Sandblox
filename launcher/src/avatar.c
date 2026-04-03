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
SDL_FColor playerColour = {1, 1, 1, 1};

AvatarItem* headAvatarItem = NULL;

AvatarItem* loadHatJson(cJSON* obj){
	AvatarItem* newItem = malloc(sizeof(AvatarItem));
	if(!newItem) return NULL;

	cJSON* mesh = cJSON_GetObjectItem(obj, "mesh");
	cJSON* texture = cJSON_GetObjectItem(obj, "texture");
	cJSON* colour = cJSON_GetObjectItem(obj, "colour");
	cJSON* graphic = cJSON_GetObjectItem(obj, "avatarImg");

	if(mesh && cJSON_IsString(mesh))
		newItem->meshPath = strdup(mesh->valuestring);
	if(texture && cJSON_IsString(texture))
		newItem->texturePath = strdup(texture->valuestring);

	SDL_Texture* tex;
	if(graphic && cJSON_IsString(graphic)){
		tex = newTexture(graphic->valuestring, SDL_SCALEMODE_LINEAR); //segment fault?
		if(!tex)
			printf("Failed to load texture from file: %s\n", graphic->valuestring);
	}
	if(tex)
		newItem->graphic = tex;

	if(colour && cJSON_IsArray(colour) && cJSON_GetArraySize(colour) >= 4)
		newItem->colour = (SDL_FColor){
			(float)(cJSON_GetArrayItem(colour, 0)->valueint) / 255,
			(float)(cJSON_GetArrayItem(colour, 1)->valueint) / 255,
			(float)(cJSON_GetArrayItem(colour, 2)->valueint) / 255,
			(float)(cJSON_GetArrayItem(colour, 3)->valueint) / 255
		};

	if(!headAvatarItem){
		headAvatarItem = newItem;
		return newItem;
	}

	AvatarItem *loopItem = headAvatarItem;
	while(loopItem->next){
		loopItem = loopItem->next;
	}
	loopItem->next = newItem;
	newItem->prev = loopItem;

	newItem->next = NULL;

	return newItem;
}

void loadAvatar(){
	char *avatarPath = malloc(256); sprintf(avatarPath, "%splayer.json", basePath);
	FILE* file = fopen(avatarPath, "r");
	if(!file){
		printf("Failed load player file %s\n", avatarPath);
		free(avatarPath); return;
	}
	free(avatarPath);

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
	}

	cJSON* name = cJSON_GetObjectItem(json, "name");
	if(name && cJSON_IsString(name))
		playerName = strdup(name->valuestring);

	cJSON* colour = cJSON_GetObjectItem(json, "colour");
	if(colour && cJSON_IsArray(colour) && cJSON_GetArraySize(colour) >= 4)
		playerColour = (SDL_FColor){
			(float)(cJSON_GetArrayItem(colour, 0)->valueint) / 255,
			(float)(cJSON_GetArrayItem(colour, 1)->valueint) / 255,
			(float)(cJSON_GetArrayItem(colour, 2)->valueint) / 255,
			(float)(cJSON_GetArrayItem(colour, 3)->valueint) / 255
		};

	cJSON* objects = cJSON_GetObjectItem(json, "objects");
		if(!objects || !cJSON_IsArray(objects)) {
		printf("No objects array found in JSON\n");
		cJSON_Delete(json);
		free(content);
		return;
	}
	
	printf("Found %d objects in JSON\n", cJSON_GetArraySize(objects));
	
	int objectCount = cJSON_GetArraySize(objects);
	for(int i = 0; i < objectCount; i++) {
		cJSON* obj = cJSON_GetArrayItem(objects, i);
		if(obj)
			loadHatJson(obj);
	}

	cJSON_Delete(json);
	free(content);
}

void initAvatar(){
	avatarBaseTex = newTexture("assets/avatar/avatarbase.png", SDL_SCALEMODE_LINEAR);

	loadAvatar();
}

void drawAvatar(SDL_Point pos){
	SDL_FRect sourceRect = (SDL_FRect){0, 0, 240, 320};
	SDL_FRect drawRect = (SDL_FRect){pos.x, pos.y, 240, 320};

	SDL_SetRenderDrawColor(renderer, 192, 192, 192, 255);
	SDL_RenderFillRect(renderer, &drawRect);

	SDL_SetTextureColorMod(avatarBaseTex, playerColour.r * 255, playerColour.g * 255, playerColour.b * 255);
	SDL_RenderTexture(renderer, avatarBaseTex, &sourceRect, &drawRect);
	AvatarItem* currItem = headAvatarItem;
	Uint8 itemIndex = 0;
	while (currItem) {
		AvatarItem *next = currItem->next;
		SDL_SetTextureColorMod(currItem->graphic, (Uint8)(currItem->colour.r * 255), (Uint8)(currItem->colour.g * 255), (Uint8)(currItem->colour.b * 255));
		SDL_RenderTexture(renderer, currItem->graphic, &sourceRect, &drawRect);
		
		SDL_SetRenderDrawColor(renderer, (Uint8)(currItem->colour.r * 255), (Uint8)(currItem->colour.g * 255), (Uint8)(currItem->colour.b * 255), (Uint8)(currItem->colour.a * 255));
		SDL_RenderFillRect(renderer, &(SDL_FRect){pos.x + 4 * itemIndex, pos.y, 4, 4});

		itemIndex++;
		currItem = next;
	}
}

#include "input.h"
void buttonRefreshAvatar(Button* item){
	(void)item;
	//return; //currently crashes

	AvatarItem* loopItem = headAvatarItem;
	while(loopItem){
		AvatarItem* currItem = loopItem;
		loopItem = loopItem->next;
		free(currItem);
	}

	headAvatarItem = NULL;
	loadAvatar();
}