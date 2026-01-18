#include "studioMain.h"
#include "studioInput.h"
#include <structs.h>

#include <SDL3/SDL.h>
#include <SDL3_image/SDL_image.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#include "../math.h"
#include "../renderer.h"

extern SDL_Window *window;
extern SDL_Renderer *renderer;
extern ClientData client;

extern double deltaTime;
extern float timer;

extern float* defaultMatrix;
extern Mesh *spherePrim;

extern DataObj *focusObject;

void drawGimble(DataObj* item);

void drawStudioOverlay(){
	drawGimble(focusObject);
}

void drawGimble(DataObj* item){
	if(!item) return;
	if(!item->classData->draw) return;
	float* objLoc = translateMatrix(defaultMatrix, (Vector3){item->pos.x, item->pos.y, item->pos.z});
	
	float* xMatrixA = translateMatrix(objLoc, (Vector3){-1.5, -item->scale.y / 2, item->scale.z / 2}); float* xMatrixB = translateMatrix(objLoc, (Vector3){item->scale.x + 1.5, -item->scale.y / 2, item->scale.z / 2});
	float* yMatrixA = translateMatrix(objLoc, (Vector3){item->scale.x / 2, 1.5, item->scale.z / 2}); float* yMatrixB = translateMatrix(objLoc, (Vector3){item->scale.x / 2, -item->scale.y - 1.5, item->scale.z / 2});
	float* zMatrixA = translateMatrix(objLoc, (Vector3){item->scale.x / 2, -item->scale.y / 2, -1.5}); float* zMatrixB = translateMatrix(objLoc, (Vector3){item->scale.x / 2, -item->scale.y / 2, item->scale.z + 1.5});
	
	drawMesh(spherePrim, xMatrixA, (SDL_FColor){1, 0, 0, 0.5}, true); drawMesh(spherePrim, xMatrixB, (SDL_FColor){1, 0, 0, 0.5}, true);
	drawMesh(spherePrim, yMatrixA, (SDL_FColor){0, 1, 0, 0.5}, true); drawMesh(spherePrim, yMatrixB, (SDL_FColor){0, 1, 0, 0.5}, true);
	drawMesh(spherePrim, zMatrixA, (SDL_FColor){0, 0, 1, 0.5}, true); drawMesh(spherePrim, zMatrixB, (SDL_FColor){0, 0, 1, 0.5}, true);
	
	free(objLoc);
	free(xMatrixA); free(xMatrixB); 
	free(yMatrixA); free(yMatrixB);
	free(zMatrixA); free(zMatrixB);
}