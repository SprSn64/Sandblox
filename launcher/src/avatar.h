#ifndef AVATAR_H
#define AVATAR_H

typedef struct AvatarItem{
	char* meshPath;
	char* texturePath;
	char* graphicPath;

	SDL_FColor colour;
	bool matchSkin;
	SDL_Texture* graphic;

	struct AvatarItem* prev;
	struct AvatarItem* next;
} AvatarItem;

void initAvatar();
void drawAvatar(SDL_Point pos);

#include "input.h"
void buttonRefreshAvatar(Button* item);
void buttonSaveAvatar(Button* item);

#endif