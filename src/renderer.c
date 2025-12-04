#include <SDL3/SDL.h>
#include <SDL3_image/SDL_image.h>

#include <stdio.h>
#include <string.h>
#include <math.h>

#include <structs.h>
#include "renderer.h"

extern SDL_Renderer *renderer;

extern float timer;
extern Camera currentCamera;

extern SDL_Point windowScaleIntent;
extern double windowScaleFactor;
extern SDL_Point windowScale;

Vector3 worldToCamera(Vector3 pos){
	return (Vector3){pos.x - currentCamera.pos.x, pos.y - currentCamera.pos.y, pos.z - currentCamera.pos.z};
}

Vector3 isoProj(Vector3 posA){
	Vector3 pos = worldToCamera(posA);
	return (Vector3){(pos.x + (pos.z/2 * SDL_cos(timer / 3.14159))) * 32 + windowScale.x / 2, (-pos.y + pos.z / 2) * 32 + windowScale.y / 2, pos.z};
	//return (Vector3){(-pos.x / (pos.z - 4)) * (windowScaleFactor * 32) + windowScale.x / 2, ((pos.y - 4) / (pos.z - 4)) * (windowScaleFactor * 32) + windowScale.y / 2, pos.z};
}

bool drawTriangle(Vector3 pointA, Vector3 pointB, Vector3 pointC, SDL_FColor colour){
	SDL_Vertex verts[3];
	verts[0].position = (SDL_FPoint){pointA.x, pointA.y}; verts[0].color = colour;
	verts[1].position = (SDL_FPoint){pointB.x, pointB.y}; verts[1].color = colour;
	verts[2].position = (SDL_FPoint){pointC.x, pointC.y}; verts[2].color = colour;
	float clockwiseAB = (verts[1].position.x - verts[0].position.x) * (verts[1].position.y + verts[0].position.y);
	float clockwiseBC = (verts[2].position.x - verts[1].position.x) * (verts[2].position.y + verts[1].position.y);
	float clockwiseCA = (verts[0].position.x - verts[2].position.x) * (verts[0].position.y + verts[2].position.y);

	if(clockwiseAB + clockwiseBC + clockwiseCA <= 0){// && max(pointA.z, max(pointB.z, pointC.z)) >= 0){
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
	if(texSurface == NULL){
		printf("Issue with loading texture %s!\n", path);
		return NULL;
	}
	
	SDL_Texture *texture = SDL_CreateTextureFromSurface(renderer, texSurface);
	SDL_SetTextureScaleMode(texture, SDL_SCALEMODE_NEAREST);
	SDL_DestroySurface(texSurface);
	return texture;
}

void drawBillboard(SDL_Texture *texture, SDL_FRect rect, Vector3 pos, SDL_FPoint offset, SDL_FPoint scale){
	Vector3 projLoc[3] = {isoProj(pos), isoProj((Vector3){pos.x--, pos.y, pos.z}), isoProj((Vector3){pos.x++, pos.y, pos.z})};
	double sizeMult = -(projLoc[2].x - projLoc[1].x) / scale.x;
	SDL_FRect sprPos = {projLoc[0].x - offset.x * sizeMult, projLoc[0].y - offset.y * sizeMult, scale.x * 4 * sizeMult, scale.y * 4 * sizeMult};
	//SDL_SetRenderDrawColor(renderer, 0, 0, 255, 255); SDL_RenderFillRect(renderer, &sprPos);
	//SDL_SetRenderDrawColor(renderer, 255, 0, 0, 255); SDL_RenderFillRect(renderer, &(SDL_FRect){projLoc[0].x - 2, projLoc[0].y - 2, 4, 4});
	//printf("%f, %f, %f, %f, %f\n", sizeMult, sprPos.x, sprPos.y, sprPos.w, sprPos.h);
	SDL_RenderTexture(renderer, texture, &rect, &sprPos);
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