#include <SDL3/SDL.h>
#include <SDL3_image/SDL_image.h>

#include <stdio.h>
#include <string.h>
#include <math.h>

#include <structs.h>
#include "renderer.h"

extern SDL_Renderer *renderer;

extern SDL_Point windowScaleIntent;
extern double windowScaleFactor;
extern SDL_Point windowScale;

SDL_FPoint isoProj(Vector3 pos){
	/*return (SDL_FPoint){
		(pos.x + pos.z/2 + windowScale.x/2) * 64, 
		(pos.z + pos.x/2 + pos.y + windowScale.y/2) * 64
	};*/
	return (SDL_FPoint){(pos.x + pos.z/2) * 64 + windowScale.x / 2, (-pos.y + pos.z / 2) * 64 + windowScale.y / 2};
}

void drawTriangle(SDL_FPoint pointA, SDL_FPoint pointB, SDL_FPoint pointC, SDL_FColor colour){
	SDL_Vertex verts[3];
	verts[0].position = pointA; verts[0].color = colour;
	verts[1].position = pointB; verts[1].color = colour;
	verts[2].position = pointC; verts[2].color = colour;
	SDL_RenderGeometry(renderer, NULL, verts, 3, NULL, 0);
}

void drawCube(Vector3 pos, Vector3 scale, SDL_FColor colour){
	//top
	drawTriangle(isoProj(pos), isoProj((Vector3){pos.x + scale.x, pos.y, pos.z}), isoProj((Vector3){pos.x + scale.x, pos.y, pos.z + scale.z}), colour);
	drawTriangle(isoProj(pos), isoProj((Vector3){pos.x, pos.y, pos.z + scale.z}), isoProj((Vector3){pos.x + scale.x, pos.y, pos.z + scale.z}), colour);
	//front
	drawTriangle(isoProj((Vector3){pos.x, pos.y, pos.z + scale.z}), isoProj((Vector3){pos.x + scale.x, pos.y, pos.z + scale.z}), isoProj((Vector3){pos.x + scale.x, pos.y - scale.y, pos.z + scale.z}), (SDL_FColor){colour.r * 0.76, colour.g * 0.8, colour.b * 0.9, 1});
	drawTriangle(isoProj((Vector3){pos.x, pos.y - scale.y, pos.z + scale.z}), isoProj((Vector3){pos.x, pos.y, pos.z + scale.z}), isoProj((Vector3){pos.x + scale.x, pos.y - scale.y, pos.z + scale.z}), (SDL_FColor){colour.r * 0.76, colour.g * 0.8, colour.b * 0.9, 1});
	//side
	drawTriangle(isoProj((Vector3){pos.x, pos.y, pos.z}), isoProj((Vector3){pos.x, pos.y, pos.z + scale.z}), isoProj((Vector3){pos.x, pos.y - scale.y, pos.z + scale.z}), (SDL_FColor){colour.r * 0.4, colour.g * 0.4, colour.b * 0.7, 1});
	drawTriangle(isoProj((Vector3){pos.x, pos.y - scale.y, pos.z}), isoProj((Vector3){pos.x, pos.y, pos.z}), isoProj((Vector3){pos.x, pos.y - scale.y, pos.z + scale.z}), (SDL_FColor){colour.r * 0.4, colour.g * 0.4, colour.b * 0.7, 1});
}