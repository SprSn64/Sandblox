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
	return (SDL_FPoint){(pos.x + pos.z/2) * 64 + windowScale.x / 2, (pos.z / 2) * 64 + windowScale.y / 2};
}

void drawCube(Vector3 pos, Vector3 scale){
	SDL_Vertex verts[4];
	verts[0].position = isoProj(pos); verts[0].color = (SDL_FColor){1, 1, 1, 1};
	verts[1].position = isoProj((Vector3){pos.x + scale.x, pos.y, pos.z}); verts[1].color = (SDL_FColor){1, 1, 1, 1};
	verts[2].position = isoProj((Vector3){pos.x + scale.x, pos.y, pos.z + scale.z}); verts[2].color = (SDL_FColor){1, 1, 1, 1};
	verts[3].position = isoProj((Vector3){pos.x, pos.y, pos.z + scale.z}); verts[3].color = (SDL_FColor){1, 1, 1, 1};
	//printf("eat my shorts!\n");
	SDL_RenderGeometry(renderer, NULL, verts, 4, NULL, 0);
	
	SDL_SetRenderDrawColor(renderer, 255, 0, 0, SDL_ALPHA_OPAQUE);
	SDL_RenderFillRect(renderer, &(SDL_FRect){verts[0].position.x - 2, verts[0].position.y - 2, 4, 4});
	SDL_RenderFillRect(renderer, &(SDL_FRect){verts[1].position.x - 2, verts[1].position.y - 2, 4, 4});
	SDL_RenderFillRect(renderer, &(SDL_FRect){verts[2].position.x - 2, verts[2].position.y - 2, 4, 4});
	SDL_RenderFillRect(renderer, &(SDL_FRect){verts[3].position.x - 2, verts[3].position.y - 2, 4, 4});
}