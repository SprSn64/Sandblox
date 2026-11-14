#include "studio.h"
#include "structs.h"

#include <SDL3/SDL.h>
#include <SDL3_image/SDL_image.h>
#include <GL/glew.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#include "../math.h"
#include "../instances.h"
#include "../renderer.h"
#include "../mesh.h"
#include "../opengl.h"

extern SDL_Window* window;
extern SDL_Point windowScale;

extern ClientData client;
extern float aspectRatio;

bool studioActive = false;

extern double deltaTime;
extern float timer;

extern float* defaultMatrix;
extern Font defaultFont;

Font studioFont;
SDL_Texture *studioFontTex = NULL;
TextureRef *classIconTex = NULL;
TextureRef *colourPickTex = NULL;
Mesh *rotateGimbleMesh = NULL;
Mesh *translateGimbleMesh = NULL;

TextureRef *toolButtonTex = NULL;

Uint32 toolMode = STUDIOTOOL_MOVE;

DataObj *focusObject = NULL;
extern SDL_MouseButtonFlags mouseState;
extern SDL_FPoint mousePos;
extern ButtonMap mouseButtons[3];
extern ButtonMap keyList[KEYBIND_MAX];

extern Mesh *spherePrim;
extern Mesh *planePrim;
extern TextureRef *cursorTex;

void initStudio(){
	//printf("Studio Initiated\n");
	if(!client.studio){
		printf("Studio not enabled!\n"); 
		return;
	}

	client.pause = true;
	
	classIconTex = loadTexture("assets/textures/studio/classicons.png", true);
	colourPickTex = loadTexture("assets/textures/studio/colourpicker.png", true);
	rotateGimbleMesh = genTorusMesh(2, 0.1, 3, 24); rotateGimbleMesh->persistent = true;
	translateGimbleMesh = loadMeshFromObj("assets/models/arrowwidget.obj", true);

	toolButtonTex = loadTexture("assets/textures/studio/toolbuttons.png", true);
}

void updateStudio(){

}

void renderStudio(){
	float* tempStudioMatrix = genMatrix((Vector3){-1, 1, 0}, (Vector3){2, 1, 2}, (Vector3){HALFPI, 0, 0});
	drawMeshOpenGL(planePrim, tempStudioMatrix, (SDL_FColor){1, 1, 1, 1}, defaultFont.texture);
	free(tempStudioMatrix);

	if(SDL_GetWindowFlags(window) & SDL_WINDOW_MOUSE_FOCUS){
		float* cursorMatrix = genMatrix(
			screenToGL((Vector3){floor(mousePos.x), floor(mousePos.y), 0}), 
			(Vector3){(32.f / windowScale.x) * aspectRatio * 2, 1, (32.f / windowScale.y) * 2}, 
			(Vector3){HALFPI, 0, 0});
		drawMeshOpenGL(planePrim, cursorMatrix, (SDL_FColor){1, 1, 1, 1}, cursorTex);
		free(cursorMatrix);
	}
}

void freeStudio(){
	if(!studioActive) return;
}

float camTime = 0;
void studioCameraUpdate(Camera* cam){
	float camSpeed = 18;
	
	Vector4 moveVec = {
		(keyList[KEYBIND_D].down - keyList[KEYBIND_A].down), 
		(keyList[KEYBIND_SPACE].down - keyList[KEYBIND_SHIFT].down), 
		(keyList[KEYBIND_S].down - keyList[KEYBIND_W].down), 
		0
	};
	
	if(moveVec.x == 0 && moveVec.y == 0 && moveVec.z == 0){
		camTime = 0;
		return;
	}
	camTime += deltaTime;
	float timeSpeedMult = camSpeed * max(sqrt(camTime) * 2, 1);
	
	float* camRotMatrix = rotateMatrix(defaultMatrix, cam->rot, ROT_XYZ);
	moveVec = matrixMult(moveVec, camRotMatrix);
	
	cam->pos = (Vector3){cam->pos.x + moveVec.x * timeSpeedMult * deltaTime, cam->pos.y + moveVec.y * timeSpeedMult * deltaTime, cam->pos.z + moveVec.z * timeSpeedMult * deltaTime};
	free(camRotMatrix);
}