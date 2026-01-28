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
#include "../instances.h"
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

extern SDL_FPoint mousePos;
extern ButtonMap mouseButtons[];

void translateGimbleUpdate(DataObj* item){
	//projToScreen(viewProj(worldToCamera(pos)))
	
	Vector3 xGimblePos[2] = {vec3Add(item->pos, (Vector3){-1.5, -item->scale.y / 2, item->scale.z / 2}), vec3Add(item->pos, (Vector3){item->scale.x + 1.5, -item->scale.y / 2, item->scale.z / 2})};
	Vector3 xGimbleProj[2] = {projToScreen(viewProj(worldToCamera(xGimblePos[0]))), projToScreen(viewProj(worldToCamera(xGimblePos[1])))};
	
	float xGimbleDist = xGimbleProj[1].x - xGimbleProj[0].x;
	SDL_FPoint xGimbleProjDist = {xGimbleProj[1].x - xGimbleProj[0].x, xGimbleProj[1].y - xGimbleProj[0].y};
	
	(void)xGimbleDist; (void)xGimbleProjDist;
	
	bool gimbleHover = (between(mousePos.x, xGimbleProj[0].x - 32, xGimbleProj[0].x + 32) && between(mousePos.y, xGimbleProj[0].y - 32, xGimbleProj[0].y + 32)) ||
			(between(mousePos.x, xGimbleProj[1].x - 32, xGimbleProj[1].x + 32) && between(mousePos.y, xGimbleProj[1].y - 32, xGimbleProj[1].y + 32));
	
	if(gimbleHover && mouseButtons[0].down){
		item->pos = vec3Add(item->pos, (Vector3){(1 - 2 * ((mousePos.x - (xGimbleProj[0].x + xGimbleProj[1].x) / 2) < 0)) * 0.2, 0, 0});
		//sendPopup("Gimble Hovered!", NULL, NULL, 1);
	}
}

void drawScaleGimble(DataObj* item){
	if(!item) return;
	if(!item->classData->draw) return;
	float* objLoc = translateMatrix(defaultMatrix, (Vector3){item->pos.x, item->pos.y, item->pos.z});
	
	float* xMatrixA = translateMatrix(objLoc, (Vector3){-1.5, -item->scale.y / 2, item->scale.z / 2}); float* xMatrixB = translateMatrix(objLoc, (Vector3){item->scale.x + 1.5, -item->scale.y / 2, item->scale.z / 2});
	float* yMatrixA = translateMatrix(objLoc, (Vector3){item->scale.x / 2, 1.5, item->scale.z / 2}); float* yMatrixB = translateMatrix(objLoc, (Vector3){item->scale.x / 2, -item->scale.y - 1.5, item->scale.z / 2});
	float* zMatrixA = translateMatrix(objLoc, (Vector3){item->scale.x / 2, -item->scale.y / 2, -1.5}); float* zMatrixB = translateMatrix(objLoc, (Vector3){item->scale.x / 2, -item->scale.y / 2, item->scale.z + 1.5});
	
	drawMesh(spherePrim, xMatrixA, (SDL_FColor){1, 0, 0, 0.5}, NULL, true); drawMesh(spherePrim, xMatrixB, (SDL_FColor){1, 0, 0, 0.5}, NULL, true);
	drawMesh(spherePrim, yMatrixA, (SDL_FColor){0, 1, 0, 0.5}, NULL, true); drawMesh(spherePrim, yMatrixB, (SDL_FColor){0, 1, 0, 0.5}, NULL, true);
	drawMesh(spherePrim, zMatrixA, (SDL_FColor){0, 0, 1, 0.5}, NULL, true); drawMesh(spherePrim, zMatrixB, (SDL_FColor){0, 0, 1, 0.5}, NULL, true);
	
	free(objLoc);
	free(xMatrixA); free(xMatrixB); 
	free(yMatrixA); free(yMatrixB);
	free(zMatrixA); free(zMatrixB);
}

void drawRotateGimble(DataObj* item){
	if(!item) return;
	if(!item->classData->draw) return;
	
	float* rotMatrix = rotateMatrix(defaultMatrix, item->rot);
	
	float* xMatrix = rotateMatrix(rotMatrix, (Vector3){0, 0, HALFPI});
	float* yMatrix = rotateMatrix(rotMatrix, (Vector3){0, HALFPI, 0});
	float* zMatrix = rotateMatrix(rotMatrix, (Vector3){HALFPI, 0, 0});
	
	free(rotMatrix);
	
	float* xPosMatrix = translateMatrix(xMatrix, item->pos);
	float* yPosMatrix = translateMatrix(yMatrix, item->pos);
	float* zPosMatrix = translateMatrix(zMatrix, item->pos);
	
	free(xMatrix); free(yMatrix); free(zMatrix);
	
	drawMesh(rotateGimbleMesh, xPosMatrix, (SDL_FColor){1, 0, 0, 0.5}, NULL, true);
	drawMesh(rotateGimbleMesh, yPosMatrix, (SDL_FColor){0, 1, 0, 0.5}, NULL, true);
	drawMesh(rotateGimbleMesh, zPosMatrix, (SDL_FColor){0, 0, 1, 0.5}, NULL, true);
	
	free(xPosMatrix); free(yPosMatrix); free(zPosMatrix);
}

void drawStudioOverlay(){
	switch(toolMode){
		case STUDIOTOOL_MOVE: drawScaleGimble(focusObject); break;
		case STUDIOTOOL_SCALE: drawScaleGimble(focusObject); break;
		case STUDIOTOOL_ROTATE: drawRotateGimble(focusObject); break;
	}
}

void updateStudioGimbles(){
	switch(toolMode){
		case STUDIOTOOL_MOVE: translateGimbleUpdate(focusObject); break;
		case STUDIOTOOL_SCALE:  break;
		case STUDIOTOOL_ROTATE:  break;
	}
}