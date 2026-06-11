#include "studio.h"
#include <structs.h>

#include <SDL3/SDL.h>
#include <SDL3_image/SDL_image.h>
#include <GL/glew.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#include "../opengl.h"
#include "../math.h"
#include "../renderer.h"

extern float aspectRatio;
extern float* defaultMatrix;
extern SDL_Point windowScale;
extern Mesh *planePrim;

StudioSplit panelHead = {false, false, NULL, NULL, 0.5, false};

void drawSplit(StudioSplit* item){
	glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

	float* aMatrix = NULL; float* bMatrix = NULL;

	if(item->vert){
		aMatrix = genMatrix((Vector3){-aspectRatio, 1, 0}, (Vector3){2 * aspectRatio, 1, 2 * item->split}, (Vector3){HALFPI, 0, 0});
		bMatrix = genMatrix((Vector3){-aspectRatio, -1 + 2 * (1-item->split), 0}, (Vector3){2 * aspectRatio, 1, 2 * (1-item->split)}, (Vector3){HALFPI, 0, 0});
	}else{
		aMatrix = genMatrix((Vector3){-aspectRatio, 1, 0}, (Vector3){aspectRatio * 2 * item->split, 1, 2}, (Vector3){HALFPI, 0, 0});
		bMatrix = genMatrix((Vector3){-aspectRatio + 2 * aspectRatio * item->split, 1, 0}, (Vector3){aspectRatio * 2 * (1-item->split), 1, 2}, (Vector3){HALFPI, 0, 0});
	}

	drawMeshOpenGL(planePrim, aMatrix, (SDL_FColor){1, 0, 0, 1}, NULL);
	drawMeshOpenGL(planePrim, bMatrix, (SDL_FColor){0, 0, 1, 1}, NULL);

	free(aMatrix); free(bMatrix);

	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
}