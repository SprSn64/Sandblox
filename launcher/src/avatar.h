#ifndef AVATAR_H
#define AVATAR_H

typedef struct AvatarItem{
	char* meshPath;
	char* texturePath;
	char* graphicPath;

	SDL_FColor colour;
	SDL_Texture* graphic;

	struct AvatarItem* prev;
	struct AvatarItem* next;
} AvatarItem;

void initAvatar();
void drawAvatar(SDL_Point pos);

#endif