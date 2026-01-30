#include "studio.h"
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
#include "../loader.h"

extern SDL_Window *window;
extern SDL_Renderer *renderer;
extern ClientData client;

SDL_Window *studioWindow = NULL;
SDL_Renderer *studioRenderer = NULL;
bool studioActive = false;

extern double deltaTime;
extern float timer;

extern float* defaultMatrix;
extern Font defaultFont;

SDL_Texture *classIconTex = NULL;
SDL_Texture *stuButtonTex = NULL;
Mesh *rotateGimbleMesh = NULL;
Mesh *translateGimbleMesh = NULL;

extern Mesh *spherePrim;

SDL_Point studioWindowScale = {240, 320};

float objListScroll = 0;
Uint32 objListLength = 0;
SDL_Rect objListRect = {32, 16, 208, 240};

Uint32 toolMode = STUDIOTOOL_NONE;

DataObj *focusObject = NULL;
extern SDL_MouseButtonFlags mouseState;
extern SDL_FPoint mousePos;
extern ButtonMap stuMouseButtons[3];

extern ButtonMap stuKeyList[STUDIOKEYBIND_MAX];
extern ButtonMap keyList[KEYBIND_MAX];

void drawObjectList(DataObj* item, int nodeDepth, int *idCount);
void drawObjectProperties(DataObj* item, int posY);

//TODO: Make button list thing for less ugly looking button implementations

Button addObjButton = {"+", (SDL_FRect){224, 304, 16, 16}, INPUTTYPE_BUTTON, buttonAddObject, true, true, false, false, NULL, &(SDL_FRect){16, 0, 16, 16}};
Button removeObjButton = {"-", (SDL_FRect){206, 304, 16, 16}, INPUTTYPE_BUTTON, buttonRemoveObject, true, true, false, false, NULL, &(SDL_FRect){32, 0, 16, 16}};
Button pauseButton = {"ll", (SDL_FRect){0, 304, 16, 16}, INPUTTYPE_BUTTON, buttonPauseGame, true, true, false, false, NULL, &(SDL_FRect){16, 32, 16, 16}};

Button saveFileButton = {"Save", (SDL_FRect){0, 0, 40, 16}, INPUTTYPE_BUTTON, buttonSaveMap, true, true, false, false, NULL, NULL};
Button loadFileButton = {"Load", (SDL_FRect){40, 0, 40, 16}, INPUTTYPE_BUTTON, buttonLoadMap, true, true, false, false, NULL, NULL};

Button selectWidgetButton = {"\0", (SDL_FRect){0, 16, 16, 16}, INPUTTYPE_BUTTON, buttonSetTool, true, true, false, false, NULL, &(SDL_FRect){0, 16, 16, 16}};
Button moveWidgetButton = {"\0", (SDL_FRect){0, 32, 16, 16}, INPUTTYPE_BUTTON, buttonSetTool, true, true, false, false, NULL, &(SDL_FRect){16, 16, 16, 16}};
Button scaleWidgetButton = {"\0", (SDL_FRect){0, 48, 16, 16}, INPUTTYPE_BUTTON, buttonSetTool, true, true, false, false, NULL, &(SDL_FRect){32, 16, 16, 16}};
Button rotateWidgetButton = {"\0", (SDL_FRect){0, 64, 16, 16}, INPUTTYPE_BUTTON, buttonSetTool, true, true, false, false, NULL, &(SDL_FRect){48, 16, 16, 16}};

void initStudio(){
	//printf("Studio Initiated\n");
	if(!client.studio){printf("Studio not enabled!\n"); return;}
	
	if(!SDL_CreateWindowAndRenderer("Studio", studioWindowScale.x, studioWindowScale.y, SDL_WINDOW_UTILITY, &studioWindow, &studioRenderer)){
		printf("Error loading studio window - %s\n", SDL_GetError()); 
		return;
	}
	studioActive = true;
	SDL_SetWindowParent(studioWindow, window);
	//SDL_SetWindowMinimumSize(studioWindow, 320, 240);
	SDL_SetRenderVSync(studioRenderer, 1);
	
	stuKeyList[STUDIOKEYBIND_DELETE].code = SDL_SCANCODE_DELETE;
	//shitty widget keys
	
	stuKeyList[STUDIOKEYBIND_Z].code = SDL_SCANCODE_Z; stuKeyList[STUDIOKEYBIND_X].code = SDL_SCANCODE_X;
	stuKeyList[STUDIOKEYBIND_C].code = SDL_SCANCODE_C; stuKeyList[STUDIOKEYBIND_V].code = SDL_SCANCODE_V;
	stuKeyList[STUDIOKEYBIND_B].code = SDL_SCANCODE_B; stuKeyList[STUDIOKEYBIND_N].code = SDL_SCANCODE_N;
	stuKeyList[STUDIOKEYBIND_D].code = SDL_SCANCODE_D;
	
	stuMouseButtons[0].code = SDL_BUTTON_LMASK; stuMouseButtons[1].code = SDL_BUTTON_MMASK; stuMouseButtons[2].code = SDL_BUTTON_RMASK;
	
	classIconTex = IMG_LoadTexture(studioRenderer, "assets/textures/classicons.png");
	stuButtonTex = IMG_LoadTexture(studioRenderer, "assets/textures/studiobuttons.png");
	rotateGimbleMesh = genTorusMesh(2, 0.1, 3, 24);
	translateGimbleMesh = loadMeshFromObj("assets/models/arrowwidget.obj");
	
	addObjButton.image = stuButtonTex;
	removeObjButton.image = stuButtonTex;
	pauseButton.image = stuButtonTex;
	
	selectWidgetButton.image = stuButtonTex;
	moveWidgetButton.image = stuButtonTex;
	scaleWidgetButton.image = stuButtonTex;
	rotateWidgetButton.image = stuButtonTex;
}

void studioCameraUpdate(Camera* cam);

void updateStudio(){
	if(!studioActive) return;
	SDL_GetWindowSize(studioWindow, &studioWindowScale.x, &studioWindowScale.y);
	
	StudioHandleKeys();
	
	SDL_SetRenderDrawColor(studioRenderer, 148, 150, 152, 255);
	SDL_RenderClear(studioRenderer);
	
	for(int i=0; i<3; i++){
		stuMouseButtons[i].down = (SDL_GetWindowFlags(studioWindow) & SDL_WINDOW_INPUT_FOCUS && (mouseState & stuMouseButtons[i].code));
		if(stuMouseButtons[i].down){
			if(!stuMouseButtons[i].pressCheck){
				stuMouseButtons[i].pressCheck = true;
				stuMouseButtons[i].pressed = true;
			}else{
				stuMouseButtons[i].pressed = false;
			}
		}else stuMouseButtons[i].pressCheck = false;
	}
	SDL_SetCursor(SDL_CreateSystemCursor(SDL_SYSTEM_CURSOR_DEFAULT));
	
	if(client.pause)
		studioCameraUpdate(client.gameWorld->currCamera);
	
	if(stuKeyList[0].pressed && focusObject){
		if(focusObject == client.gameWorld->currPlayer)
			client.gameWorld->currPlayer = NULL;
		removeObject(focusObject);
		focusObject = NULL;
	}
	
	/*Vector3 gimbleAddVec = {
		(float)(stuKeyList[STUDIOKEYBIND_X].pressed - stuKeyList[STUDIOKEYBIND_Z].pressed) / 2, 
		(float)(stuKeyList[STUDIOKEYBIND_V].pressed - stuKeyList[STUDIOKEYBIND_C].pressed) / 2, 
		(float)(stuKeyList[STUDIOKEYBIND_N].pressed - stuKeyList[STUDIOKEYBIND_B].pressed) / 2
	};
	switch(toolMode){
		case STUDIOTOOL_MOVE: focusObject->pos = vec3Add(focusObject->pos, gimbleAddVec); break;
		case STUDIOTOOL_SCALE: focusObject->scale = vec3Add(focusObject->scale, gimbleAddVec); break;
		case STUDIOTOOL_ROTATE: focusObject->rot = vec3Add(focusObject->rot, (Vector3){gimbleAddVec.x * HALFPI / 3, gimbleAddVec.y * HALFPI / 3, gimbleAddVec.z * HALFPI / 3}); break;
	}*/
	
	if(stuKeyList[STUDIOKEYBIND_V].pressed && focusObject){
		char* clipboardText = SDL_GetClipboardText();
		
		if(clipboardText[0] == '#'){
			char rHex[3]; sprintf(rHex, "%c%c", clipboardText[1], clipboardText[2]); focusObject->colour.r = (Uint8)strtol(rHex, NULL, 16);
			char gHex[3]; sprintf(gHex, "%c%c", clipboardText[3], clipboardText[4]); focusObject->colour.g = (Uint8)strtol(gHex, NULL, 16);
			char bHex[3]; sprintf(bHex, "%c%c", clipboardText[5], clipboardText[6]); focusObject->colour.b = (Uint8)strtol(bHex, NULL, 16);
			
			//strtol(str, &endptr, 10);
		}
		
		free(clipboardText);
	}
	if(stuKeyList[STUDIOKEYBIND_C].pressed && focusObject){
		char colourHex[8]; sprintf(colourHex, "#%x%x%x", focusObject->colour.r, focusObject->colour.g, focusObject->colour.b);
		SDL_SetClipboardText(colourHex);
	}
	
	if(stuKeyList[STUDIOKEYBIND_D].pressed && focusObject)
		focusObject = duplicateObject(focusObject);
	
	int idCounter = 0;
	drawObjectList(client.gameWorld->headObj, 0, &idCounter);
	
	if(objListLength > floor(objListRect.h / 16) + 2){
		SDL_SetRenderDrawColor(studioRenderer, 192, 193, 196, SDL_ALPHA_OPAQUE);
		float scrollHeight = objListRect.h / max(1, (objListLength - floor(objListRect.h / 16)));
		float scrollT = min(1, max(0, objListScroll / (objListLength - objListRect.h / 16)));
		SDL_RenderFillRect(studioRenderer, &(SDL_FRect){objListRect.x + objListRect.w - 8, objListRect.y + (objListRect.h - scrollHeight) * scrollT, 8, scrollHeight});
	}
	
	if(focusObject)drawStudioOverlay();
	
	//make this less shitty soon
	updateAndDrawButton(studioRenderer, &addObjButton); updateAndDrawButton(studioRenderer, &removeObjButton); updateAndDrawButton(studioRenderer, &pauseButton);
	updateAndDrawButton(studioRenderer, &loadFileButton); updateAndDrawButton(studioRenderer, &saveFileButton);
	updateAndDrawButton(studioRenderer, &selectWidgetButton); updateAndDrawButton(studioRenderer, &moveWidgetButton); updateAndDrawButton(studioRenderer, &scaleWidgetButton); updateAndDrawButton(studioRenderer, &rotateWidgetButton);
	
	drawObjectProperties(focusObject, 240);
	
	SDL_RenderPresent(studioRenderer);
}

void studioCleanup(){
	if(!studioActive) return;
	SDL_DestroyTexture(classIconTex); SDL_DestroyTexture(stuButtonTex); 
	free(rotateGimbleMesh); free(translateGimbleMesh);
}

void drawObjectList(DataObj* item, int nodeDepth, int *idCount){	
	int i = (*idCount)++;
	float itemYOffset = (i - objListScroll) * 16;
	if(closest(itemYOffset - 8, 16) >= objListRect.h - 32) 
		return;
	
	if(itemYOffset < 0)
		goto listRenderSkip;
	
	if(between(mousePos.x, objListRect.x, objListRect.x + objListRect.w) && between(mousePos.y, objListRect.y + 3 + itemYOffset, objListRect.y + 15 + itemYOffset) && stuMouseButtons[0].pressed){
		if(item == focusObject){
			item->studioOpen = !item->studioOpen;
		}
		focusObject = item;
		goto focusSkip;
	}
	if(focusObject == item){
		focusSkip:
		SDL_SetRenderDrawColor(studioRenderer, 64, 192, 24, SDL_ALPHA_OPAQUE);
		
		SDL_RenderFillRect(studioRenderer, &(SDL_FRect){objListRect.x, objListRect.y + itemYOffset, objListRect.w, 16});
	}
	
	SDL_SetRenderDrawColor(studioRenderer, 255, 255, 255, 255);
	SDL_RenderDebugText(studioRenderer, objListRect.x + 18/**/ + (nodeDepth * 24), 20/**/ + itemYOffset, item->name);
	drawText(studioRenderer, &defaultFont, item->name, objListRect.x + 18/**/ + (nodeDepth * 24), 20/**/ + itemYOffset, 1, (SDL_FColor){1, 1, 1, 1}); //why no render????
	
	SDL_FRect iconRect = {(item->classData->id % 16) * 16, (int)floor((float)item->classData->id / 16) * 16 % 256, 16, 16};
	SDL_FRect iconPos = {objListRect.x + nodeDepth * 24, objListRect.y/**/ + itemYOffset, 16, 16};
	SDL_RenderTexture(studioRenderer, classIconTex, &iconRect, &iconPos);
	
	if(!item->studioOpen && item->child)
		SDL_RenderTexture(studioRenderer, classIconTex, &(SDL_FRect){245, 249, 11, 7}, &(SDL_FRect){objListRect.x + nodeDepth * 24, objListRect.y + itemYOffset, 11, 7});
	
	listRenderSkip:
	
	if(!item->studioOpen) return;
	
	DataObj* child = item->child;
	while (child) {
		DataObj *next = child->next;
		drawObjectList(child, nodeDepth + 1, idCount);
		child = next;
	}
}

void drawObjectProperties(DataObj* item, int posY){
	//excuse the slop
	char string[256];
	
	SDL_SetRenderDrawColor(studioRenderer, 255, 255, 255, 255);
	
	if(!item){
		SDL_RenderDebugText(studioRenderer, 2, posY, "No object selected!"); return;
	}
	
	sprintf(string, "Name: %s", item->name);
	SDL_RenderDebugText(studioRenderer, 2, posY, string);
	sprintf(string, "Class: %s", item->classData->name);
	SDL_RenderDebugText(studioRenderer, 2, posY + 8, string);
	sprintf(string, "Position: %.2f, %.2f, %.2f", item->pos.x, item->pos.y, item->pos.z);
	SDL_RenderDebugText(studioRenderer, 2, posY + 16, string);
	sprintf(string, "Rotation: %d, %d, %d", (int)(item->rot.x * RAD2DEG), (int)(item->rot.y * RAD2DEG), (int)(item->rot.z * RAD2DEG));
	SDL_RenderDebugText(studioRenderer, 2, posY + 24, string);
	sprintf(string, "Scale: %.2f, %.2f, %.2f", item->scale.x, item->scale.y, item->scale.z);
	SDL_RenderDebugText(studioRenderer, 2, posY + 32, string);
	
	SDL_RenderDebugText(studioRenderer, 2, posY + 40, "Colour: ");
	SDL_SetRenderDrawColor(studioRenderer, item->colour.r, item->colour.g, item->colour.b, 255); 
	SDL_RenderFillRect(studioRenderer, &(SDL_FRect){64, posY + 40, 24, 8});
	SDL_SetRenderDrawColor(studioRenderer, item->colour.r * item->colour.a / 255, item->colour.g * item->colour.a / 255, item->colour.b * item->colour.a / 255, 255); 
	SDL_RenderFillRect(studioRenderer, &(SDL_FRect){88, posY + 40, 24, 8});
}

void studioCameraUpdate(Camera* cam){
	float camSpeed = 18;
	static float camTime = 0;
	
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
	float timeSpeedMult = camSpeed * max(sqrt(camTime), 1);
	
	float* camRotMatrix = rotateMatrix(defaultMatrix, cam->rot);
	moveVec = matrixMult(moveVec, camRotMatrix);
	
	cam->pos = (Vector3){cam->pos.x + moveVec.x * timeSpeedMult * deltaTime, cam->pos.y + moveVec.y * timeSpeedMult * deltaTime, cam->pos.z + moveVec.z * timeSpeedMult * deltaTime};
	free(camRotMatrix);
}