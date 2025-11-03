#include <SDL3/SDL.h>
#include <SDL3_image/SDL_image.h>

#include <stdio.h>
#include <string.h>
#include <math.h>

#include <structs.h>
#include "renderer.h"

extern SDL_Renderer *renderer;

extern float timer;

extern SDL_Point windowScaleIntent;
extern double windowScaleFactor;
extern SDL_Point windowScale;

SDL_FPoint isoProj(Vector3 pos){
	/*return (SDL_FPoint){
		(pos.x + pos.z/2 + windowScale.x/2) * 64, 
		(pos.z + pos.x/2 + pos.y + windowScale.y/2) * 64
	};*/
	return (SDL_FPoint){(pos.x + (pos.z/2 * SDL_cos(timer / 3.14159))) * 64 + windowScale.x / 2, (-pos.y + pos.z / 2) * 64 + windowScale.y / 2};
	//return (SDL_FPoint){(-pos.x / (pos.z - 4)) * (windowScaleFactor * 64) + windowScale.x / 2, ((pos.y - 4) / (pos.z - 4)) * (windowScaleFactor * 64) + windowScale.y / 2};
}

bool drawTriangle(SDL_FPoint pointA, SDL_FPoint pointB, SDL_FPoint pointC, SDL_FColor colour){
	SDL_Vertex verts[3];
	verts[0].position = pointA; verts[0].color = colour;
	verts[1].position = pointB; verts[1].color = colour;
	verts[2].position = pointC; verts[2].color = colour;
	float clockwiseAB = (verts[1].position.x - verts[0].position.x) * (verts[1].position.y + verts[0].position.y);
	float clockwiseBC = (verts[2].position.x - verts[1].position.x) * (verts[2].position.y + verts[1].position.y);
	float clockwiseCA = (verts[0].position.x - verts[2].position.x) * (verts[0].position.y + verts[2].position.y);

	if(clockwiseAB + clockwiseBC + clockwiseCA <= 0){
		SDL_RenderGeometry(renderer, NULL, verts, 3, NULL, 0);
		return 0;
	}
	return 1;
}

void drawCube(Vector3 pos, Vector3 scale, SDL_FColor colour){
	//side
	drawTriangle(isoProj((Vector3){pos.x, pos.y, pos.z}), isoProj((Vector3){pos.x, pos.y, pos.z + scale.z}), isoProj((Vector3){pos.x, pos.y - scale.y, pos.z + scale.z}), (SDL_FColor){colour.r * 0.4, colour.g * 0.4, colour.b * 0.7, 1});
	drawTriangle(isoProj((Vector3){pos.x, pos.y - scale.y, pos.z}), isoProj((Vector3){pos.x, pos.y, pos.z}), isoProj((Vector3){pos.x, pos.y - scale.y, pos.z + scale.z}), (SDL_FColor){colour.r * 0.4, colour.g * 0.4, colour.b * 0.7, 1});
	//front
	drawTriangle(isoProj((Vector3){pos.x, pos.y, pos.z + scale.z}), isoProj((Vector3){pos.x + scale.x, pos.y, pos.z + scale.z}), isoProj((Vector3){pos.x + scale.x, pos.y - scale.y, pos.z + scale.z}), (SDL_FColor){colour.r * 0.76, colour.g * 0.8, colour.b * 0.9, 1});
	drawTriangle(isoProj((Vector3){pos.x, pos.y - scale.y, pos.z + scale.z}), isoProj((Vector3){pos.x, pos.y, pos.z + scale.z}), isoProj((Vector3){pos.x + scale.x, pos.y - scale.y, pos.z + scale.z}), (SDL_FColor){colour.r * 0.76, colour.g * 0.8, colour.b * 0.9, 1});
	//top
	drawTriangle(isoProj(pos), isoProj((Vector3){pos.x + scale.x, pos.y, pos.z}), isoProj((Vector3){pos.x + scale.x, pos.y, pos.z + scale.z}), colour);
	drawTriangle(isoProj((Vector3){pos.x, pos.y, pos.z + scale.z}), isoProj(pos), isoProj((Vector3){pos.x + scale.x, pos.y, pos.z + scale.z}), colour);
}

SDL_Texture *newTexture(char* path){
	SDL_Surface *texSurface = NULL;
	
	texSurface = IMG_Load(path);
	
	SDL_Texture *texture = SDL_CreateTextureFromSurface(renderer, texSurface);
	SDL_SetTextureScaleMode(texture, SDL_SCALEMODE_NEAREST);
	SDL_DestroySurface(texSurface);
	return texture;
}

void drawText(SDL_Renderer *renderer, SDL_Texture *texture, char* text, char charOff, short posX, short posY, short width, short height, short kern){
	for(int i=0; i<=strlen(text); i++){
		char charVal = (unsigned)text[i] - charOff;
		int xOff = (charVal % 16) * width;
		int yOff = floor((float)charVal / 16) * height;
		SDL_FRect sprRect = {xOff, yOff, width, height};
		SDL_FRect sprPos = {posX + kern * i, posY, width, height};
		SDL_RenderTexture(renderer, texture, &sprRect, &sprPos);
	}
}