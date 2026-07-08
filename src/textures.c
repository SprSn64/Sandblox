#include <SDL3/SDL.h>
#include <GL/glew.h>

#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#include "mesh.h"
#include "opengl.h"
#include "textures.h"

TextureRef* headTexture = NULL;

extern SDL_Renderer* renderer;
SDL_Texture *newTexture(char* path, SDL_ScaleMode scaleMode){
	SDL_Texture *texture = IMG_LoadTexture(renderer, path);
	if(texture == NULL){
		printf("Issue with loading SDL texture %s!\n", path);
		return NULL;
	}
	SDL_SetTextureScaleMode(texture, scaleMode);
	return texture;
}

Texture* newRasterTexture(Uint16 width, Uint16 height){
	Texture* newTexture = malloc(sizeof(Texture));
	if(!newTexture){
		printf("Failed to generate texture of size %dx%d\n", width, height);
		return NULL;
	}
	newTexture->width = width; newTexture->height = height;
	newTexture->pixels = malloc(width * height * sizeof(Uint32));
	
	if(!newTexture->pixels){
	    free(newTexture);
	    return NULL;
	}
	
	printf("Succesfully made texture of size %dx%d\n", width, height);
	return newTexture;
}

bool freeRasterTexture(Texture* tex){
	if(!tex) return 1;
	free(tex->pixels); free(tex);
	return 0;
}

Texture* loadRasterTexture(char* path){
	SDL_Surface* newSurface = IMG_Load(path); if(!newSurface) return NULL;
	Texture* newTex = newRasterTexture(newSurface->w, newSurface->h);

	memcpy(newTex->pixels, newSurface->pixels, (Uint32)newSurface->w*newSurface->h * sizeof(Uint32)); 

	SDL_DestroySurface(newSurface);
	return newTex;
}

TextureRef* textureExists(char* path){
	TextureRef *loopItem = headTexture;
	while(loopItem){
		if(!strcmp(loopItem->filePath, path))
			return loopItem;
		loopItem = loopItem->next;
	}
	return NULL;
}

TextureRef* loadTexture(char* path, bool persistent){
	TextureRef* texCheck = textureExists(path);
	if(texCheck){
		printf("Texture %s already exists...\n", path);
		return texCheck;
	}

	TextureRef* texItem = calloc(1, sizeof(TextureRef));
	if(!texItem) return NULL;
	texItem->filePath = strdup(path);
	texItem->persistent = persistent;

	Texture* texture = loadRasterTexture(path);
	if(!texture) {
	    free(texItem->filePath);
	    free(texItem);
	    return NULL;
	}
	texItem->texture = texture;

	glGenTextures(1, &texItem->glLoc);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, texItem->glLoc);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, texture->width, texture->height, 0, GL_RGBA, GL_UNSIGNED_BYTE, texture->pixels);
	glGenerateMipmap(GL_TEXTURE_2D);

	glBindTexture(GL_TEXTURE_2D, 0);

	if(!headTexture){
		headTexture = texItem;
		return texItem;
	}

	TextureRef *loopItem = headTexture;
	while(loopItem->next){
		loopItem = loopItem->next;
	}
	texItem->prev = loopItem;
	loopItem->next = texItem;

	return texItem;
}

void updateGlTexture(TextureRef* tex){
	glBindTexture(GL_TEXTURE_2D, tex->glLoc);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, tex->texture->width, tex->texture->height, 0, GL_RGBA, GL_UNSIGNED_BYTE, tex->texture->pixels);
	glBindTexture(GL_TEXTURE_2D, 0);
}

void freeTexture(TextureRef* tex){
	if(!tex) return;
	if(tex->image)SDL_DestroyTexture(tex->image);
	if(tex->texture)freeRasterTexture(tex->texture);
	if(tex->glLoc)glDeleteTextures(1, &tex->glLoc);
	if(tex->filePath)free(tex->filePath);

	if(headTexture == tex)
		headTexture = tex->next;

	if(tex->next)tex->next->prev = tex->prev;
	if(tex->prev)tex->prev->next = tex->next;
	free(tex);
}

void cleanupTextures(bool soft){
	TextureRef* currItem = headTexture;
	while (currItem) {
		if(soft && currItem->persistent){
			currItem = currItem->next;
			continue;
		}

		TextureRef *next = currItem->next;
		freeTexture(currItem); 
		currItem = next;
	}
}