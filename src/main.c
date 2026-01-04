#define SDL_MAIN_USE_CALLBACKS 1
#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>
#include <SDL3_image/SDL_image.h>

#define GLFW_DLL
#define GLFW_INCLUDE_NONE
#include <glad/gl.h>
#include <GLFW/glfw3.h>
//#include <iostream>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#include <structs.h>
#include "instances.h"
#include "renderer.h"
#include "math.h"
#include "loader.h"

#include "studio/studio.h"

SDL_Window *window = NULL;
SDL_Window *glWindow = NULL;
SDL_Renderer *renderer = NULL;

bool glEnabled = false;
Uint32 glVersion[2] = {0, 0};

SDL_Point windowScaleIntent = {320, 240};
double windowScaleFactor;
SDL_Point windowScale = {640, 480};

Camera currentCamera = {(Vector3){0, 2, 10}, (Vector3){0, 0, 0}, 90, 1, 16, NULL, NULL};

Uint64 last = 0;
Uint64 now = 0;
double deltaTime = 0;
Uint32 lastFPS = 0;

float timer = 0;

SDL_Texture *fontTex = NULL;
SDL_Texture *playerTex = NULL;
SDL_Texture *homerTex = NULL;

Mesh *teapotMesh = NULL;
Mesh *playerMesh = NULL;
Mesh *cubeMesh = NULL;

Mesh *cubePrim = NULL;
Mesh *spherePrim = NULL;

KeyMap keyList[KEYBINDCOUNT];

void HandleKeyInput();

extern float renderScale;

extern DataType playerClass;
extern DataType fuckingBeerdrinkerClass;
extern DataType blockClass;
extern DataObj gameHeader;

extern Vector3 lightNormal;

DataObj* playerObj = NULL;
DataObj* blockAObj = NULL;
DataObj* blockBObj = NULL;

SDL_AppResult SDL_AppInit(void **appstate, int argc, char *argv[]){
	SDL_SetAppMetadata("SandBlox", "0.0", NULL);
	
	for(int i=0; i < argc; i++){
		printf("%s\n", argv[i]); 
		glEnabled = !strcmp("-opengl", argv[i]);
	}
	
	if(!SDL_Init(SDL_INIT_VIDEO)){
		SDL_Log("Couldn't initialize SDL: %s", SDL_GetError());
		return SDL_APP_FAILURE;
	}

	if(!SDL_CreateWindowAndRenderer("Sandblox (3D Software)", windowScale.x, windowScale.y, SDL_WINDOW_RESIZABLE, &window, &renderer)){
		SDL_Log("Couldn't create window/renderer: %s", SDL_GetError());
		return SDL_APP_FAILURE;
	}
	if(glEnabled){
		glfwInit();
		glWindow = SDL_CreateWindow("Sandblox (3D OpenGL)", windowScale.x, windowScale.y, SDL_WINDOW_RESIZABLE | SDL_WINDOW_OPENGL);
		SDL_SetWindowParent(glWindow, window); //SDL_SetWindowModal(glWindow, true);
		SDL_SetWindowMinimumSize(glWindow, 320, 240);
		
		glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
		glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
		glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
		
		//printf("OpenGl %s\n", glGetString(GL_VERSION));
	}
	
	SDL_SetWindowMinimumSize(window, 320, 240);
	
	fontTex = newTexture("assets/textures/font.png", SDL_SCALEMODE_NEAREST);
	playerTex = newTexture("assets/textures/playertemp.png", SDL_SCALEMODE_NEAREST);
	homerTex = newTexture("assets/textures/homer.png", SDL_SCALEMODE_NEAREST);

	teapotMesh = loadMeshFromObj("assets/models/teapot.obj");
	playerMesh = loadMeshFromObj("assets/models/oldplayer.obj");
	cubeMesh = loadMeshFromObj("assets/models/testcube.obj");
	
	cubePrim = loadMeshFromObj("assets/models/primitives/cube.obj");
	spherePrim = loadMeshFromObj("assets/models/primitives/sphere.obj");
	
	SDL_SetRenderVSync(renderer, 1);
	
	currentCamera.transform = perspMatrix(90, 4/3, 0.01, 100);

	keyList[KEYBIND_W].scanCode = SDL_SCANCODE_W; keyList[KEYBIND_S].scanCode = SDL_SCANCODE_S; keyList[KEYBIND_A].scanCode = SDL_SCANCODE_A; keyList[KEYBIND_D].scanCode = SDL_SCANCODE_D;
	keyList[KEYBIND_SPACE].scanCode = SDL_SCANCODE_SPACE; keyList[KEYBIND_SHIFT].scanCode = SDL_SCANCODE_LSHIFT;
	keyList[KEYBIND_UP].scanCode = SDL_SCANCODE_UP; keyList[KEYBIND_DOWN].scanCode = SDL_SCANCODE_DOWN; keyList[KEYBIND_LEFT].scanCode = SDL_SCANCODE_LEFT; keyList[KEYBIND_RIGHT].scanCode = SDL_SCANCODE_RIGHT;
	keyList[KEYBIND_I].scanCode = SDL_SCANCODE_I; keyList[KEYBIND_O].scanCode = SDL_SCANCODE_O;

	playerObj = newObject(NULL, &playerClass);
	newObject(NULL, &fuckingBeerdrinkerClass);
	blockAObj = newObject(NULL, &blockClass);
	blockAObj->pos = (Vector3){-1, 2, -1}; blockAObj->scale = (Vector3){2, 2, 2}; blockAObj->colour = (CharColour){153, 204, 255, 255};
	blockAObj->name = "BlueBlock\0";
	blockBObj = newObject(blockAObj, &blockClass);
	blockBObj->pos = (Vector3){6, 5, 0}; blockBObj->scale = (Vector3){1, 1, 1}; blockBObj->colour = (CharColour){255, 51, 76, 255};
	blockBObj->name = "RedBlock\0";

	if (playerObj == NULL) {
		printf("player is fucking null\n");
		exit(1);
	}

	return SDL_APP_CONTINUE;
}	

SDL_AppResult SDL_AppEvent(void *appstate, SDL_Event *event){
	if(event->type == SDL_EVENT_QUIT){
		return SDL_APP_SUCCESS;
	}
	return SDL_APP_CONTINUE;
}

SDL_AppResult SDL_AppIterate(void *appstate){
	HandleKeyInput();
	
	last = now;
	now = SDL_GetTicks();
	deltaTime = min(((double)now - (double)last) / 1000.0f, 1);
	timer += deltaTime;
	
	SDL_GetWindowSize(window, &windowScale.x, &windowScale.y);
	windowScaleFactor = min((float)windowScale.x / windowScaleIntent.x, (float)windowScale.y / windowScaleIntent.y);
	renderScale = min(windowScale.x, windowScale.y);
	
	SDL_SetRenderDrawColor(renderer, 20, 22, 24, SDL_ALPHA_OPAQUE);
	SDL_RenderClear(renderer);
	
	//lightNormal = (Vector3){SDL_cos(timer / 2), SDL_sin(timer / 2), 0};
	
	//currentCamera.pos.x += ((SDL_cos(currentCamera.rot.y) * (keyList[KEYBIND_D].down - keyList[KEYBIND_A].down)) + (SDL_sin(currentCamera.rot.y) * (keyList[KEYBIND_S].down - keyList[KEYBIND_W].down))) * 2 * deltaTime;
	//currentCamera.pos.y += (keyList[KEYBIND_SPACE].down - keyList[KEYBIND_SHIFT].down) * 2 * deltaTime;
	//currentCamera.pos.z += ((-SDL_sin(currentCamera.rot.y) * (keyList[KEYBIND_D].down - keyList[KEYBIND_A].down)) + (SDL_cos(currentCamera.rot.y) * (keyList[KEYBIND_S].down - keyList[KEYBIND_W].down))) * 2 * deltaTime;
	
	currentCamera.rot.x += (keyList[KEYBIND_UP].down - keyList[KEYBIND_DOWN].down) * 1 * deltaTime;
	currentCamera.rot.y += (keyList[KEYBIND_LEFT].down - keyList[KEYBIND_RIGHT].down) * 1 * deltaTime;
	currentCamera.rot = (Vector3){fmod(currentCamera.rot.x, 360 * DEG2RAD), fmod(currentCamera.rot.y, 360 * DEG2RAD), fmod(currentCamera.rot.z, 360 * DEG2RAD)};
	
	currentCamera.focusDist = min(max(currentCamera.focusDist + (keyList[KEYBIND_O].down - keyList[KEYBIND_I].down) * 4 * sqrt(currentCamera.focusDist + 1) * deltaTime, 0), 64);
	
	int idCounter = 0;
	updateObjects(&gameHeader, 0, &idCounter, false);
	
	//drawCube((Vector3){(2 + SDL_cos(timer)) / -2, SDL_sin(timer) + 1, (2 + SDL_cos(timer)) / -2}, (Vector3){2 + SDL_cos(timer), SDL_sin(timer) + 1, 2 + SDL_cos(timer)}, (SDL_FColor){0.6, 0.8, 1, 1});
	//drawCube((Vector3){SDL_sin(timer) * 2 - 0.5, 1, SDL_cos(timer) * 2 - 0.5}, (Vector3){1, 1, 1}, (SDL_FColor){1, 0.2, 0.3, 1});
	SDL_SetRenderDrawColor(renderer, 255, 255, 255, SDL_ALPHA_OPAQUE);
	
	idCounter = 0;
	updateObjects(&gameHeader, 0, &idCounter, true);
	
	char guiText[256];
	if ((Uint32)(timer*100)%64 == 0) {
		lastFPS = (Uint32)floor(1/deltaTime);
	}
	sprintf(guiText, "FPS: %d", lastFPS);
	drawText(renderer, fontTex, guiText, 32, 0, 0, 16, 16, 12);
	sprintf(guiText, "Camera Rot: %d, %d", (int)(currentCamera.rot.y * RAD2DEG), (int)(currentCamera.rot.x * RAD2DEG));
	drawText(renderer, fontTex, guiText, 32, 0, 16, 16, 16, 12);
	//drawText(renderer, fontTex, "Diagnostics: Skill issue", 32, 0, 64, 16, 16, 12);
	//SDL_RenderDebugText(renderer, 0, 0, guiText);

	SDL_RenderPresent(renderer);

	return SDL_APP_CONTINUE;
}

void SDL_AppQuit(void *appstate, SDL_AppResult result){
	cleanupObjects(&gameHeader);
	SDL_DestroyTexture(fontTex);
}
    
void HandleKeyInput(){
	const bool* keyState = SDL_GetKeyboardState(NULL);
	for(int i = 0; i < KEYBINDCOUNT; i++){
		keyList[i].down = keyState[keyList[i].scanCode];
	}
}