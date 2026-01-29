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
float ogGimbles[2];
Vector3 ogProjs[2];
char gimbleGrabbed = 0;
void translateGimbleUpdate(DataObj* item){
	//projToScreen(viewProj(worldToCamera(pos)))
	if(!(SDL_GetWindowFlags(window) & SDL_WINDOW_INPUT_FOCUS)) return;
	
	//code spaghetti.... yum!
	
	Vector3 xPos[2] = {vec3Add(item->pos, (Vector3){-1.5, -item->scale.y / 2, item->scale.z / 2}), vec3Add(item->pos, (Vector3){item->scale.x + 1.5, -item->scale.y / 2, item->scale.z / 2})};
	Vector3 xProj[2] = {projToScreen(viewProj(worldToCamera(xPos[0]))), projToScreen(viewProj(worldToCamera(xPos[1])))};
	float xScale[2] = {1 / xProj[0].z * renderScale, 1 / xProj[1].z * renderScale};
	bool xHoverA = between(mousePos.x, xProj[0].x + xScale[0] / 2, xProj[0].x - xScale[0] / 2) && between(mousePos.y, xProj[0].y + xScale[0] / 2, xProj[0].y - xScale[0] / 2);
	bool xHoverB = between(mousePos.x, xProj[1].x + xScale[1] / 2, xProj[1].x - xScale[1] / 2) && between(mousePos.y, xProj[1].y + xScale[1] / 2, xProj[1].y - xScale[1] / 2);
	
	Vector3 yPos[2] = {vec3Add(item->pos, (Vector3){item->scale.x / 2, 1.5, item->scale.z / 2}), vec3Add(item->pos, (Vector3){item->scale.x / 2, -item->scale.y - 1.5, item->scale.z / 2})};
	Vector3 yProj[2] = {projToScreen(viewProj(worldToCamera(yPos[0]))), projToScreen(viewProj(worldToCamera(yPos[1])))};
	float yScale[2] = {1 / yProj[0].z * renderScale, 1 / yProj[1].z * renderScale};
	bool yHoverA = between(mousePos.x, yProj[0].x + yScale[0] / 2, yProj[0].x - yScale[0] / 2) && between(mousePos.y, yProj[0].y + yScale[0] / 2, yProj[0].y - yScale[0] / 2);
	bool yHoverB = between(mousePos.x, yProj[1].x + yScale[1] / 2, yProj[1].x - yScale[1] / 2) && between(mousePos.y, yProj[1].y + yScale[1] / 2, yProj[1].y - yScale[1] / 2);
	
	Vector3 zPos[2] = {vec3Add(item->pos, (Vector3){item->scale.z / 2, -item->scale.y / 2, -1.5}), vec3Add(item->pos, (Vector3){item->scale.x /2, -item->scale.y / 2, item->scale.z + 1.5})};
	Vector3 zProj[2] = {projToScreen(viewProj(worldToCamera(zPos[0]))), projToScreen(viewProj(worldToCamera(zPos[1])))};
	float zScale[2] = {1 / zProj[0].z * renderScale, 1 / zProj[1].z * renderScale};
	bool zHoverA = between(mousePos.x, zProj[0].x + zScale[0] / 2, zProj[0].x - zScale[0] / 2) && between(mousePos.y, zProj[0].y + zScale[0] / 2, zProj[0].y - zScale[0] / 2);
	bool zHoverB = between(mousePos.x, zProj[1].x + zScale[1] / 2, zProj[1].x - zScale[1] / 2) && between(mousePos.y, zProj[1].y + zScale[1] / 2, zProj[1].y - zScale[1] / 2);
	
	if(xProj[0].z >= 0 && xProj[1].z >= 0 && yProj[0].z >= 0 && yProj[1].z >= 0 && zProj[0].z >= 0 && zProj[1].z >= 0) return;
	
	if(!mouseButtons[0].down)
		gimbleGrabbed = 0;
	
	SDL_FPoint distVec; float dist; float angle; SDL_FPoint mouseDist; float dragDist;
	
	if(gimbleGrabbed){
		distVec = (SDL_FPoint){ogProjs[1].x - ogProjs[0].x, ogProjs[1].y - ogProjs[0].y};
		dist = ogGimbles[1] - ogGimbles[0];
		
		angle = atan2(distVec.y, distVec.x);
		mouseDist = (SDL_FPoint){mousePos.x - ogProjs[0].x, mousePos.y - ogProjs[0].y};
		dragDist = (SDL_cos(angle) * mouseDist.x + SDL_sin(angle) * mouseDist.y) / sqrt(distVec.x * distVec.x + distVec.y * distVec.y);
	}
	
	switch(gimbleGrabbed){
		case 1: item->pos.x = closest(ogPos.x + dist * dragDist, 1); break;
		case 2: item->pos.y = closest(ogPos.y + dist * dragDist, 1); break;
		case 3: item->pos.z = closest(ogPos.z + dist * dragDist, 1); break;
	}
	
	if(mouseButtons[0].pressed && gimbleGrabbed == 0){
		//sendPopup("X gimble pressed!", NULL, NULL, 3);
		if(xHoverA || xHoverB){
			ogGimbles[0] = xPos[0].x; ogGimbles[1] = xPos[1].x;
			ogProjs[0] = xProj[0]; ogProjs[1] = xProj[1];
			gimbleGrabbed = 1;
			return;
		}
		if(yHoverA || yHoverB){
			ogGimbles[0] = yPos[0].y; ogGimbles[1] = yPos[1].y;
			ogProjs[0] = yProj[0]; ogProjs[1] = yProj[1];
			gimbleGrabbed = 2;
			return;
		}
		if(zHoverA || zHoverB){
			ogGimbles[0] = zPos[0].z; ogGimbles[1] = zPos[1].z;
			ogProjs[0] = zProj[0]; ogProjs[1] = zProj[1];
			gimbleGrabbed = 3;
			return;
		}
		
		ogPos = item->pos;
	}
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