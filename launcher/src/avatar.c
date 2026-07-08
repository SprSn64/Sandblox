#include "structs.h"
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
SDL_Texture* avatarFemBaseTex = NULL;

extern Font defaultFont;
SDL_Texture *newTexture(char* path, SDL_ScaleMode scaleMode);
void drawText(SDL_Renderer *renderLoc, Font *textFont, char* text, short posX, short posY, float scale, SDL_FColor colour);

char* playerName;
bool playerFem = false;
SDL_FColor playerColour = {1, 1, 1, 1};

AvatarItem* headAvatarItem = NULL;

AvatarItem* loadHatJson(cJSON* obj){
	AvatarItem* newItem = malloc(sizeof(AvatarItem));
	if(!newItem) return NULL;

	cJSON* name = cJSON_GetObjectItem(obj, "name");
	cJSON* mesh = cJSON_GetObjectItem(obj, "mesh");
	cJSON* texture = cJSON_GetObjectItem(obj, "texture");
	cJSON* colour = cJSON_GetObjectItem(obj, "colour");
	cJSON* graphic = cJSON_GetObjectItem(obj, "avatarImg");
	cJSON* matchSkin = cJSON_GetObjectItem(obj, "matchSkin");

	if(mesh && cJSON_IsString(mesh))
		newItem->meshPath = strdup(mesh->valuestring);
	if(texture && cJSON_IsString(texture))
		newItem->texturePath = strdup(texture->valuestring);

	SDL_Texture* tex;
	if(graphic && cJSON_IsString(graphic)){
		tex = newTexture(graphic->valuestring, SDL_SCALEMODE_LINEAR);
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

	if(matchSkin && cJSON_IsBool(matchSkin))
		newItem->matchSkin = cJSON_IsTrue(matchSkin);
	else
		newItem->matchSkin = false;

	if(name && cJSON_IsString(name))
		printf("Loaded hat %s.\n", name->valuestring);
	else
		printf("Loaded unnamed hat.\n");

	newItem->prev = NULL;
	newItem->next = NULL;

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
		free(content);
		return;
	}

	cJSON* name = cJSON_GetObjectItem(json, "name");
	if(name && cJSON_IsString(name))
		playerName = strdup(name->valuestring);

	playerFem = false;
	cJSON* femBody = cJSON_GetObjectItem(json, "femBody");
      if(femBody && cJSON_IsBool(femBody) && cJSON_IsTrue(femBody)){
      	playerFem = true;
      }

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
	avatarFemBaseTex = newTexture("assets/avatar/avatarbasefem.png", SDL_SCALEMODE_LINEAR);

	loadAvatar();
}

void setDrawColor(SDL_Renderer* render, SDL_FColor colour){
	SDL_SetRenderDrawColor(render, (Uint8)(colour.r * 255), (Uint8)(colour.g * 255), (Uint8)(colour.b * 255), (Uint8)(colour.a * 255));
}

void setTextureColor(SDL_Texture* texture, SDL_FColor colour){
	SDL_SetTextureColorMod(texture, (Uint8)(colour.r * 255), (Uint8)(colour.g * 255), (Uint8)(colour.b * 255));
}

void drawAvatar(SDL_Point pos){
	SDL_FRect sourceRect = (SDL_FRect){0, 0, 240, 320};
	SDL_FRect drawRect = (SDL_FRect){pos.x, pos.y, 240, 320};

	SDL_SetRenderDrawColor(renderer, 192, 192, 192, 255);
	SDL_RenderFillRect(renderer, &drawRect);

	if(playerFem){
		setTextureColor(avatarFemBaseTex, playerColour);
		SDL_RenderTexture(renderer, avatarFemBaseTex, &sourceRect, &drawRect);
	}else{
		setTextureColor(avatarBaseTex, playerColour);
		SDL_RenderTexture(renderer, avatarBaseTex, &sourceRect, &drawRect);
	}
	
	AvatarItem* currItem = headAvatarItem;
	//Uint8 itemIndex = 0;
	while (currItem) {
		AvatarItem *next = currItem->next;
		if(currItem->matchSkin)
			setTextureColor(currItem->graphic, (SDL_FColor){
				currItem->colour.r * playerColour.r,
				currItem->colour.b * playerColour.g,
				currItem->colour.g * playerColour.b,
				1
			});
		else
			setTextureColor(currItem->graphic, currItem->colour);
		SDL_RenderTexture(renderer, currItem->graphic, &sourceRect, &drawRect);
		
		//setDrawColor(renderer, currItem->colour);
		//SDL_RenderFillRect(renderer, &(SDL_FRect){pos.x + 4 * itemIndex, pos.y, 4, 4});

		//itemIndex++;
		currItem = next;
	}
}

#include "input.h"
void buttonRefreshAvatar(Button* item){
	(void)item;

	AvatarItem* loopItem = headAvatarItem;
	while(loopItem){
		AvatarItem* currItem = loopItem;
		loopItem = loopItem->next;
		if(currItem->graphic)SDL_DestroyTexture(currItem->graphic);
		free(currItem);
	}

	headAvatarItem = NULL;
	loadAvatar();
}

void buttonSaveAvatar(Button* item){
	(void)item;
}