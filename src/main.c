#define SDL_MAIN_USE_CALLBACKS 1
#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>
#include <SDL3_image/SDL_image.h>
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
#include "map.h"
#include "opengl.h"
#include "gamefile.h"
#include "server.h"

#include "studio/studio.h"

extern SDL_Window *studioWindow;

SDL_Window *window = NULL;
SDL_Renderer *renderer = NULL;

bool glEnabled = false;
Uint32 glVersion[2] = {0, 0};

ClientData client;
GameWorld game;

//SDL_Point windowScaleIntent = {320, 240};
//double windowScaleFactor;
SDL_Point windowScale = {640, 480};

Camera currentCamera = {(Vector3){0, 2, 10}, (Vector3){0, 0, 0}, 90, 1, 16, NULL, NULL};
Uint8 camMoveMode = 0;
float mouseSense = 0.2;

bool mapLoaded = false;
bool gameFileLoaded = false;

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

ButtonMap keyList[KEYBINDCOUNT];

SDL_MouseButtonFlags mouseState;
SDL_FPoint mousePos;
SDL_FPoint storedMousePos;
ButtonMap mouseButtons[3];

void HandleKeyInput();

extern float renderScale;

//insert better mesh initiating system here or somewhere
extern DataType playerClass;
extern DataType fuckingBeerdrinkerClass;
extern DataType blockClass;
extern DataType meshClass;

extern DataObj gameHeader;
extern DataObj *focusObject;

extern Vector3 lightNormal;

DataObj* playerObj = NULL;

SDL_AppResult SDL_AppInit(void **appstate, int argc, char *argv[]){
	(void)appstate;
	SDL_SetAppMetadata("SandBlox", "0.0", NULL);
	
	for(int i=0; i < argc; i++){
		//printf("%s\n", argv[i]); 
		if(!strcmp("-opengl", argv[i]))glEnabled = true;
		if(!strcmp("-debug", argv[i]))client.debug = true;
		if(!strcmp("-studio", argv[i]))client.studio = true;
		
		if(!strcmp("-mapfile", argv[i])) {
			const char *mapName = argv[++i];
			printf("Opening map %s\n", mapName);
			loadMapFromSBMap(mapName);
			mapLoaded = true;
		}
		if(!strcmp("-server", argv[i])) printf("cant join server '%s'... not implemented yet sorry\n", argv[i+1]);
			//connectServer(argv[i++]);
	}
	
	if(!SDL_Init(SDL_INIT_VIDEO)){
		SDL_Log("Couldn't initialize SDL: %s", SDL_GetError());
		return SDL_APP_FAILURE;
	}

	if(!SDL_CreateWindowAndRenderer("Sandblox (3D Software)", windowScale.x, windowScale.y, SDL_WINDOW_RESIZABLE, &window, &renderer)){
		SDL_Log("Couldn't create window/renderer: %s", SDL_GetError());
		return SDL_APP_FAILURE;
	}
	if(glEnabled)
		glEnabled = initOpenGL();
	SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
	SDL_SetWindowMinimumSize(window, 320, 240);
	SDL_SetRenderVSync(renderer, 1);
	
	fontTex = newTexture("assets/textures/font.png", SDL_SCALEMODE_NEAREST);
	playerTex = newTexture("assets/textures/playertemp.png", SDL_SCALEMODE_NEAREST);
	homerTex = newTexture("assets/textures/homer.png", SDL_SCALEMODE_NEAREST);

	teapotMesh = loadMeshFromObj("assets/models/teapot.obj");
	playerMesh = loadMeshFromObj("assets/models/oldplayer.obj"); //will be replaced with a better model soon
	cubeMesh = loadMeshFromObj("assets/models/testcube.obj");
	
	cubePrim = loadMeshFromObj("assets/models/primitives/cube.obj");
	spherePrim = loadMeshFromObj("assets/models/primitives/sphere.obj");

	keyList[KEYBIND_W].code = SDL_SCANCODE_W; keyList[KEYBIND_S].code = SDL_SCANCODE_S; keyList[KEYBIND_A].code = SDL_SCANCODE_A; keyList[KEYBIND_D].code = SDL_SCANCODE_D;
	keyList[KEYBIND_SPACE].code = SDL_SCANCODE_SPACE; keyList[KEYBIND_SHIFT].code = SDL_SCANCODE_LSHIFT;
	keyList[KEYBIND_UP].code = SDL_SCANCODE_UP; keyList[KEYBIND_DOWN].code = SDL_SCANCODE_DOWN; keyList[KEYBIND_LEFT].code = SDL_SCANCODE_LEFT; keyList[KEYBIND_RIGHT].code = SDL_SCANCODE_RIGHT;
	keyList[KEYBIND_I].code = SDL_SCANCODE_I; keyList[KEYBIND_O].code = SDL_SCANCODE_O;
	
	mouseButtons[0].code = SDL_BUTTON_LMASK; mouseButtons[1].code = SDL_BUTTON_MMASK; mouseButtons[2].code = SDL_BUTTON_RMASK;
	
	lightNormal = normalize3(lightNormal);
	
	initStudio();

	client.gameWorld = &game;
	client.gameWorld->headObj = &gameHeader;
	gameHeader.studioOpen = true;

	client.gameWorld->currPlayer = NULL;
	client.gameWorld->currCamera = &currentCamera;

	if(mapLoaded) return SDL_APP_CONTINUE;
	
	if(loadGameFile("assets/gamefile.json") == 0){
		printf("Successfully loaded gamefile\n");
		gameFileLoaded = true;
	} else {
		printf("Failed to load gamefile\n");
		gameFileLoaded = false;
	}
	
	focusObject = client.gameWorld->currPlayer;
	
	return SDL_APP_CONTINUE;
}	

SDL_AppResult SDL_AppEvent(void *appstate, SDL_Event *event){
	(void)appstate;
	if(event->type == SDL_EVENT_QUIT){
		return SDL_APP_SUCCESS;
	}

	if(event->type == SDL_EVENT_MOUSE_WHEEL){
		bool mainFocus = SDL_GetWindowFlags(window) & SDL_WINDOW_INPUT_FOCUS;
		bool studioFocus = studioWindow && (SDL_GetWindowFlags(studioWindow) & SDL_WINDOW_INPUT_FOCUS);
		
		if(mainFocus || studioFocus){
			float zoomSpeed = 0.1f;
			float zoomMin = 0.1f;
			float zoomMax = 5.0f;
			
			if(event->wheel.y > 0){
				currentCamera.zoom += zoomSpeed;
			} else if(event->wheel.y < 0){
				currentCamera.zoom -= zoomSpeed;
			}
			
			if(currentCamera.zoom < zoomMin) currentCamera.zoom = zoomMin;
			if(currentCamera.zoom > zoomMax) currentCamera.zoom = zoomMax;
		}
	}
	
	return SDL_APP_CONTINUE;
}

SDL_AppResult SDL_AppIterate(void *appstate){
	(void)appstate;
	HandleKeyInput();
	
	mouseState = SDL_GetMouseState(&mousePos.x, &mousePos.y);
	for(int i=0; i<3; i++){
		mouseButtons[i].down = (SDL_GetWindowFlags(window) & SDL_WINDOW_INPUT_FOCUS && (mouseState & mouseButtons[i].code));
		if(mouseButtons[i].down){
			if(!mouseButtons[i].pressCheck){
				mouseButtons[i].pressCheck = true;
				mouseButtons[i].pressed = true;
			}else{
				mouseButtons[i].pressed = false;
			}
		}else mouseButtons[i].pressCheck = false;
	}
	
	last = now;
	now = SDL_GetTicks();
	deltaTime = min(((double)now - (double)last) / 1000.0f, 1);
	timer += deltaTime;
	
	SDL_GetWindowSize(window, &windowScale.x, &windowScale.y);
	//windowScaleFactor = min((float)windowScale.x / windowScaleIntent.x, (float)windowScale.y / windowScaleIntent.y);
	renderScale = min(windowScale.x, windowScale.y);
	
	SDL_SetRenderDrawColor(renderer, 20, 22, 24, SDL_ALPHA_OPAQUE);
	SDL_RenderClear(renderer);
	
	updateStudio();
	
	/*for(int i = 0; i < playerMesh->vertCount; i++){
		playerMesh->verts[i].pos.x += (1 - SDL_randf() * 2) * 0.002;
		playerMesh->verts[i].pos.y += (1 - SDL_randf() * 2) * 0.002;
		playerMesh->verts[i].pos.z += (1 - SDL_randf() * 2) * 0.002;
	}*/
	
	//lightNormal = (Vector3){SDL_cos(timer / 2), SDL_sin(timer / 2), 0};
	
	//currentCamera.pos.x += ((SDL_cos(currentCamera.rot.y) * (keyList[KEYBIND_D].down - keyList[KEYBIND_A].down)) + (SDL_sin(currentCamera.rot.y) * (keyList[KEYBIND_S].down - keyList[KEYBIND_W].down))) * 2 * deltaTime;
	//currentCamera.pos.y += (keyList[KEYBIND_SPACE].down - keyList[KEYBIND_SHIFT].down) * 2 * deltaTime;
	//currentCamera.pos.z += ((-SDL_sin(currentCamera.rot.y) * (keyList[KEYBIND_D].down - keyList[KEYBIND_A].down)) + (SDL_cos(currentCamera.rot.y) * (keyList[KEYBIND_S].down - keyList[KEYBIND_W].down))) * 2 * deltaTime;
	
	SDL_ShowCursor();
	bool mainWindowFocus = SDL_GetWindowFlags(window) & SDL_WINDOW_INPUT_FOCUS;
	if(mouseButtons[2].down && mainWindowFocus){
		if(camMoveMode){
			currentCamera.rot.x += -(mousePos.y - storedMousePos.y) * mouseSense * deltaTime;
			currentCamera.rot.y += -(mousePos.x - storedMousePos.x) * mouseSense * deltaTime;  
			SDL_WarpMouseInWindow(window, storedMousePos.x, storedMousePos.y); SDL_HideCursor();
		}else{
			storedMousePos = mousePos;
			camMoveMode = 1;
		}
	}else camMoveMode = 0;
	
	currentCamera.rot.x += (keyList[KEYBIND_UP].down - keyList[KEYBIND_DOWN].down) * 1 * deltaTime;
	currentCamera.rot.y += (keyList[KEYBIND_LEFT].down - keyList[KEYBIND_RIGHT].down) * 1 * deltaTime;
	currentCamera.rot = (Vector3){fmod(currentCamera.rot.x, 6.28318), fmod(currentCamera.rot.y, 6.28318), fmod(currentCamera.rot.z, 6.28318)};
	currentCamera.focusDist = min(max(currentCamera.focusDist + (keyList[KEYBIND_I].down - keyList[KEYBIND_O].down) * 4 * fmax(1, sqrt(currentCamera.focusDist)) * deltaTime, 0), 64);
	
	int idCounter = 0;
	updateObjects(&gameHeader, 0, &idCounter, false);
	
	//drawCube((Vector3){(2 + SDL_cos(timer)) / -2, SDL_sin(timer) + 1, (2 + SDL_cos(timer)) / -2}, (Vector3){2 + SDL_cos(timer), SDL_sin(timer) + 1, 2 + SDL_cos(timer)}, (SDL_FColor){0.6, 0.8, 1, 1});
	//drawCube((Vector3){SDL_sin(timer) * 2 - 0.5, 1, SDL_cos(timer) * 2 - 0.5}, (Vector3){1, 1, 1}, (SDL_FColor){1, 0.2, 0.3, 1});
	SDL_SetRenderDrawColor(renderer, 255, 255, 255, SDL_ALPHA_OPAQUE);
	
	idCounter = 0;
	updateObjects(&gameHeader, 0, &idCounter, true);
	
	//renderTriList();
	
	static char fpsText[256] = "FPS: 0";
	static char rotText[256] = "Camera Rot: 0, 0";
	static double lastDebugUpdate = 0;
	if ((Uint32)(timer*100)%64 == 0) {
		lastFPS = (Uint32)floor(1/deltaTime);
	}
	if(timer - lastDebugUpdate >= 0.5){
		lastDebugUpdate = timer;
		sprintf(fpsText, "FPS: %d", lastFPS);
		sprintf(rotText, "Camera Rot: %d, %d", (int)(currentCamera.rot.y * RAD2DEG), (int)(currentCamera.rot.x * RAD2DEG));
	}
	drawText(renderer, fontTex, fpsText, 32, 0, 0, 16, 16, 12);
	drawText(renderer, fontTex, rotText, 32, 0, 16, 16, 16, 12);
	
	if(client.debug)drawText(renderer, fontTex, "Debug Enabled", 32, 0, windowScale.y - 16, 16, 16, 12);
	if(client.studio)drawText(renderer, fontTex, "Studio Enabled", 32, 0, windowScale.y - 32, 16, 16, 12);
	
	if(!gameFileLoaded) {
		char* noGameText = "NO GAME HERE";
		int textWidth = strlen(noGameText) * 12;
		int centerX = (windowScale.x - textWidth) / 2;
		int centerY = windowScale.y / 2;
		drawText(renderer, fontTex, noGameText, 32, centerX, centerY, 16, 16, 12);
	}
	
	//drawText(renderer, fontTex, "Diagnostics: Skill issue", 32, 0, 64, 16, 16, 12);
	//SDL_RenderDebugText(renderer, 0, 0, guiText);
	if(glEnabled)
		updateOpenGL();
	
	SDL_RenderPresent(renderer);

	return SDL_APP_CONTINUE;
}

void SDL_AppQuit(void *appstate, SDL_AppResult result){
	(void)appstate; (void)result;
	cleanupObjects(&gameHeader);
	SDL_DestroyTexture(fontTex);
}
    
void HandleKeyInput(){
	const bool* keyState = SDL_GetKeyboardState(NULL);
	bool hasFocus = (SDL_GetWindowFlags(window) & SDL_WINDOW_INPUT_FOCUS) || 
	                (studioWindow && (SDL_GetWindowFlags(studioWindow) & SDL_WINDOW_INPUT_FOCUS));
	for(int i = 0; i < KEYBINDCOUNT; i++){
		keyList[i].down = keyState[keyList[i].code] && hasFocus;
		if(keyList[i].down){
			if(!keyList[i].pressCheck){
				keyList[i].pressCheck = true;
				keyList[i].pressed = true;
			}else{
				keyList[i].pressed = false;
			}
		}else keyList[i].pressCheck = false;
	}
}