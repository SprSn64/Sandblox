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

extern Mesh *rotateGimbleMesh;

extern Uint32 toolMode;
extern DataObj *focusObject;

void drawScaleGimble(DataObj* item){
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

void drawRotateGimble(DataObj* item){
	if(!item) return;
	if(!item->classData->draw) return;
	Vector3 centerPos = {item->pos.x + item->scale.x / 2, item->pos.y + item->scale.y / 2, item->pos.z + item->scale.z / 2};
	
	float* xMatrix = translateMatrix(rotateMatrix(defaultMatrix, (Vector3){item->rot.x, item->rot.y, PI / 2}), centerPos);
	float* yMatrix = translateMatrix(rotateMatrix(defaultMatrix, (Vector3){item->rot.x, PI / 2, item->rot.z}), centerPos);
	float* zMatrix = translateMatrix(rotateMatrix(defaultMatrix, (Vector3){PI / 2, item->rot.y, item->rot.z}), centerPos);
	
	drawMesh(rotateGimbleMesh, xMatrix, (SDL_FColor){1, 0, 0, 0.5}, true);
	drawMesh(rotateGimbleMesh, yMatrix, (SDL_FColor){0, 1, 0, 0.5}, true);
	drawMesh(rotateGimbleMesh, zMatrix, (SDL_FColor){0, 0, 1, 0.5}, true);
	
	free(xMatrix); free(yMatrix); free(zMatrix);
}

void drawStudioOverlay(){
	switch(toolMode){
		case STUDIOTOOL_MOVE:
		case STUDIOTOOL_SCALE: drawScaleGimble(focusObject); break;
		case STUDIOTOOL_ROTATE: drawRotateGimble(focusObject); break;
	}
}