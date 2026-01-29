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
extern Mesh *translateGimbleMesh;

extern Uint32 toolMode;
extern DataObj *focusObject;

extern SDL_FPoint mousePos;
extern ButtonMap mouseButtons[];
extern float renderScale;

Vector3 ogPos;
float ogXGimbles[2];
Vector3 ogXProjs[2];
bool gimbleGrabbed = false;
void translateGimbleUpdate(DataObj* item){
	//projToScreen(viewProj(worldToCamera(pos)))
	if(!(SDL_GetWindowFlags(window) & SDL_WINDOW_INPUT_FOCUS)) return;
	
	Vector3 xPos[2] = {vec3Add(item->pos, (Vector3){-1.5, -item->scale.y / 2, item->scale.z / 2}), vec3Add(item->pos, (Vector3){item->scale.x + 1.5, -item->scale.y / 2, item->scale.z / 2})};
	Vector3 xProj[2] = {projToScreen(viewProj(worldToCamera(xPos[0]))), projToScreen(viewProj(worldToCamera(xPos[1])))};
	if(xProj[0].z >= 0 && xProj[1].z >= 0) return;
	
	float xScale[2] = {1 / xProj[0].z * renderScale, 1 / xProj[1].z * renderScale};
	//printf("%f, %f\n", xScale[0], xScale[1]);
	//printf("%f, %f\n", xProj[0].x, xProj[1].x);
	
	bool xHoverA = between(mousePos.x, xProj[0].x + xScale[0] / 2, xProj[0].x - xScale[0] / 2) && between(mousePos.y, xProj[0].y + xScale[0] / 2, xProj[0].y - xScale[0] / 2);
	bool xHoverB = between(mousePos.x, xProj[1].x + xScale[1] / 2, xProj[1].x - xScale[1] / 2) && between(mousePos.y, xProj[1].y + xScale[1] / 2, xProj[1].y - xScale[1] / 2);
	
	//SDL_SetRenderDrawColor(renderer, 0, 255, 255, SDL_ALPHA_OPAQUE); 
	
	if(!mouseButtons[0].down){
		gimbleGrabbed = false;
	}
	
	if(mouseButtons[0].pressed && !gimbleGrabbed && (xHoverA || xHoverB)){
		sendPopup("X gimble pressed!", NULL, NULL, 3);
		ogPos = item->pos;
		ogXGimbles[0] = xPos[0].x; ogXGimbles[1] = xPos[1].x;
		ogXProjs[0] = xProj[0]; ogXProjs[1] = xProj[1];
		gimbleGrabbed = true;
		//return;
	}
	
	if(gimbleGrabbed){
		SDL_FPoint xDistVec = {ogXProjs[1].x - ogXProjs[0].x, ogXProjs[1].y - ogXProjs[0].y};
		float xDist = ogXGimbles[1] - ogXGimbles[0];
		
		float xAngle = atan2(xDistVec.y, xDistVec.x);
		SDL_FPoint mouseDist = {mousePos.x - ogXProjs[0].x, mousePos.y - ogXProjs[0].y};
		float dragDist = (SDL_cos(xAngle) * mouseDist.x + SDL_sin(xAngle) * mouseDist.y) / sqrt(xDistVec.x * xDistVec.x + xDistVec.y * xDistVec.y);
		item->pos.x = ogPos.x + xDist * dragDist;
		
		return;
	}
	
	/*if(xHoverA + xHoverB > 0){
		SDL_SetRenderDrawColor(renderer, 255, 255, 255, SDL_ALPHA_OPAQUE); 
	}
	
	SDL_RenderFillRect(renderer, &(SDL_FRect){xProj[0].x - xScale[0]/2, xProj[0].y - xScale[0]/2, xScale[0], xScale[0]});
	SDL_RenderFillRect(renderer, &(SDL_FRect){xProj[1].x - xScale[1]/2, xProj[1].y - xScale[1]/2, xScale[1], xScale[1]});*/
}

void drawTranslateGimble(DataObj* item){
	if(!item) return;
	if(!item->classData->draw) return;
	Vector3 itemCenter = vec3Add(item->pos, vec3Mult(item->scale, (Vector3){0.5, -0.5, 0.5}));
	
	float* xMatrixA = genMatrix(vec3Add(itemCenter, (Vector3){-item->scale.x / 2, 0, 0}), (Vector3){1, 1, 1}, (Vector3){0, 0, HALFPI});
	float* xMatrixB = genMatrix(vec3Add(itemCenter, (Vector3){item->scale.x / 2, 0, 0}), (Vector3){1, 1, 1}, (Vector3){0, 0, -HALFPI});
	drawMesh(translateGimbleMesh, xMatrixA, (SDL_FColor){1, 0, 0, 0.5}, NULL, true); drawMesh(translateGimbleMesh, xMatrixB, (SDL_FColor){1, 0, 0, 0.5}, NULL, true);
	free(xMatrixA); free(xMatrixB);
	
	float* yMatrixA = genMatrix(vec3Add(itemCenter, (Vector3){0, -item->scale.y / 2, 0}), (Vector3){1, -1, 1}, (Vector3){0, 0, 0});
	float* yMatrixB = genMatrix(vec3Add(itemCenter, (Vector3){0, item->scale.y / 2, 0}), (Vector3){1, 1, 1}, (Vector3){0, 0, 0});
	drawMesh(translateGimbleMesh, yMatrixA, (SDL_FColor){0, 1, 0, 0.5}, NULL, true); drawMesh(translateGimbleMesh, yMatrixB, (SDL_FColor){0, 1, 0, 0.5}, NULL, true);
	free(yMatrixA); free(yMatrixB);
	
	float* zMatrixA = genMatrix(vec3Add(itemCenter, (Vector3){0, 0, -item->scale.z / 2}), (Vector3){1, 1, 1}, (Vector3){-HALFPI, 0, 0});
	float* zMatrixB = genMatrix(vec3Add(itemCenter, (Vector3){0, 0, item->scale.z / 2}), (Vector3){1, 1, 1}, (Vector3){HALFPI, 0, 0});
	drawMesh(translateGimbleMesh, zMatrixA, (SDL_FColor){0, 0, 1, 0.5}, NULL, true); drawMesh(translateGimbleMesh, zMatrixB, (SDL_FColor){0, 0, 1, 0.5}, NULL, true);
	free(zMatrixA); free(zMatrixB);
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
		case STUDIOTOOL_MOVE: drawTranslateGimble(focusObject); break;
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