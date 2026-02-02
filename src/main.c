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
#include "opengl.h"
#include "gamefile.h"
#include "server.h"

#include "studio/studio.h"

/* TODO:
	Get OpenGL GLEW working
	Make studio widgets work properly (1/3 complete)
	Add multiplayer server shit
	Implement simple physics
	Add secondary studio menu for changing object properties (colour, scale, etc)
*/

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

extern SDL_Rect objListRect;
extern float objListScroll;
extern Uint32 objListLength;

Camera currentCamera = {(Vector3){0, 2, 10}, (Vector3){0, 0, 0}, 90, 1, 16, NULL, NULL};
Uint8 camMoveMode = 0;
float mouseSense = 0.1;

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

Font defaultFont;

SDL_Texture *cowTex = NULL;
SDL_Texture *skyTex = NULL;
SDL_Texture *sunTex = NULL;

Mesh *playerMesh = NULL;
Mesh *skyboxMesh = NULL;
Mesh *sunMesh = NULL;

Mesh *planePrim = NULL;
Mesh *cubePrim = NULL;
Mesh *spherePrim = NULL;

ButtonMap keyList[KEYBIND_MAX];

SDL_MouseButtonFlags mouseState;
SDL_FPoint mousePos;
SDL_FPoint storedMousePos;
ButtonMap mouseButtons[3];
Uint8 camResetTimer = 0;

void HandleKeyInput();

extern float renderScale;

//insert better mesh initiating system here or somewhere
extern DataType playerClass;
extern DataType fuckingBeerdrinkerClass;
extern DataType blockClass;
extern DataType meshClass;
extern DataType groupClass;

extern DataObj gameHeader;
extern DataObj *focusObject;

extern Vector3 lightNormal;
extern SDL_FColor lightColour;

DataObj* playerObj = NULL;

float* defaultMatrix = NULL;
float* skyboxMatrix = NULL;
float* sunMatrix = NULL;
SDL_FColor skyboxColour = {0.8, 0.82, 1, 1};
Vector3 sunAngle = {0, 0, 0};

SDL_AppResult SDL_AppInit(void **appstate, int argc, char *argv[]){
	(void)appstate;
	client.version = "0.0";
	SDL_SetAppMetadata("SandBlox", client.version, NULL);
	
	char *mapToLoad = "assets/gamefile.json";
	
	for(int i=0; i < argc; i++){
		//printf("%s\n", argv[i]); 
		if(!strcmp("-opengl", argv[i]))glEnabled = true;
		if(!strcmp("-debug", argv[i]))client.debug = true;
		if(!strcmp("-studio", argv[i]))client.studio = true;
		
		if(!strcmp("-mapfile", argv[i]))
			mapToLoad = argv[++i];
		if(!strcmp("-server", argv[i])) printf("cant join server '%s'... not implemented yet sorry\n", argv[i+1]);
			//connectServer(argv[i++]);
	}
	
	if(!SDL_Init(SDL_INIT_VIDEO)){
		SDL_Log("Couldn't initialize SDL: %s", SDL_GetError());
		return SDL_APP_FAILURE;
	}

	char windowName[64] = "Sandblox vXX.XX (3D Software)";
	sprintf(windowName, "Sandblox v%s (3D Software)", client.version);
	if(!SDL_CreateWindowAndRenderer(windowName, windowScale.x, windowScale.y, SDL_WINDOW_RESIZABLE, &window, &renderer)){
		SDL_Log("Couldn't create window/renderer: %s", SDL_GetError());
		return SDL_APP_FAILURE;
	}
	SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
	SDL_SetWindowMinimumSize(window, 320, 240);
	SDL_SetRenderVSync(renderer, 1);
	
	fontTex = newTexture("assets/textures/font.png", SDL_SCALEMODE_NEAREST);
	playerTex = newTexture("assets/textures/playertemp.png", SDL_SCALEMODE_NEAREST);
	homerTex = newTexture("assets/textures/homer.png", SDL_SCALEMODE_NEAREST);
	cowTex = newTexture("assets/textures/cows.png", SDL_SCALEMODE_LINEAR);
	skyTex = newTexture("assets/textures/skybox.png", SDL_SCALEMODE_LINEAR);
	sunTex = newTexture("assets/textures/sunflare.png", SDL_SCALEMODE_LINEAR);
	
	defaultFont = (Font){fontTex, 32, (SDL_Point){8, 8}, (SDL_FPoint){6, 0}, 16};

	playerMesh = loadMeshFromObj("assets/models/player.obj"); //will be replaced with a better model soon
	skyboxMesh = loadMeshFromObj("assets/models/advskybox.obj");
	sunMesh = loadMeshFromObj("assets/models/skyboxsun.obj");
	
	planePrim = genPlaneMesh(1, 1, 1, 1);
	cubePrim = loadMeshFromObj("assets/models/primitives/cube.obj");
	spherePrim = loadMeshFromObj("assets/models/primitives/sphere.obj");
	
	if(glEnabled)
		glEnabled = initOpenGL();

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
	
	defaultMatrix = newMatrix();

	if(mapLoaded) return SDL_APP_CONTINUE;
	
	if(loadGameFile(mapToLoad) == 0){
		gameFileLoaded = true;
	} else {
		printf("Failed to load gamefile\n");
		sendPopup("Failed to load gamefile", NULL, NULL, 3);
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
		
		if(mainFocus && !client.pause){
			float zoomSpeed = max(1, sqrt(currentCamera.focusDist));
			float zoomMin = 0.0f;
			float zoomMax = 64.0f;
			
			currentCamera.focusDist += zoomSpeed * (1 - 2 * (event->wheel.y > 0)) * (event->wheel.y != 0);
			currentCamera.focusDist = min(max(currentCamera.focusDist, zoomMin), zoomMax);
		}
		if(studioFocus && between(mousePos.x, objListRect.x, objListRect.x + objListRect.w) && between(mousePos.y, objListRect.y, objListRect.y + objListRect.h)){
			float scrollSpeed = 1.0f;
			objListScroll += scrollSpeed * (1 - 2 * (event->wheel.y > 0)) * (event->wheel.y != 0);
			objListScroll = min(max(objListScroll, 0), max(objListLength - floor(objListRect.h / 16) + 2, 0));
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
	
	//setDrawColour(renderer, skyboxColour);
	//SDL_SetRenderDrawColor(renderer, skyboxColour.r * 255, skyboxColour.g * 255, skyboxColour.b * 255, SDL_ALPHA_OPAQUE);
	//SDL_RenderClear(renderer);
	
	/*for(int i = 0; i < playerMesh->vertCount; i++){
		playerMesh->verts[i].pos.x += (1 - SDL_randf() * 2) * 0.002;
		playerMesh->verts[i].pos.y += (1 - SDL_randf() * 2) * 0.002;
		playerMesh->verts[i].pos.z += (1 - SDL_randf() * 2) * 0.002;
	}*/
	
	//currentCamera.pos.x += ((SDL_cos(currentCamera.rot.y) * (keyList[KEYBIND_D].down - keyList[KEYBIND_A].down)) + (SDL_sin(currentCamera.rot.y) * (keyList[KEYBIND_S].down - keyList[KEYBIND_W].down))) * 2 * deltaTime;
	//currentCamera.pos.y += (keyList[KEYBIND_SPACE].down - keyList[KEYBIND_SHIFT].down) * 2 * deltaTime;
	//currentCamera.pos.z += ((-SDL_sin(currentCamera.rot.y) * (keyList[KEYBIND_D].down - keyList[KEYBIND_A].down)) + (SDL_cos(currentCamera.rot.y) * (keyList[KEYBIND_S].down - keyList[KEYBIND_W].down))) * 2 * deltaTime;
	
	SDL_ShowCursor();
	bool mainWindowFocus = SDL_GetWindowFlags(window) & SDL_WINDOW_INPUT_FOCUS;
	if(currentCamera.focusDist == 0 && !client.pause){
		if(camMoveMode != 2){
			storedMousePos = mousePos;
			camMoveMode = 2;
		}
		goto firstPerson;
	}
	if(mouseButtons[2].down && mainWindowFocus){
		if(camMoveMode == 1){
			firstPerson:
			float moveScale = mouseSense / (renderScale / windowScale.x);
			currentCamera.rot.x += -(mousePos.y - storedMousePos.y) * moveScale * deltaTime;
			currentCamera.rot.y += -(mousePos.x - storedMousePos.x) * moveScale * deltaTime;  
			camResetTimer++;
			if(camResetTimer >= 4){
				SDL_WarpMouseInWindow(window, storedMousePos.x, storedMousePos.y); 
				camResetTimer = 0;
			}
			SDL_HideCursor();
		}else{
			storedMousePos = mousePos;
			camMoveMode = 1;
		}
	}else camMoveMode = 0;
	
	currentCamera.rot.x += (keyList[KEYBIND_UP].down - keyList[KEYBIND_DOWN].down) * 1 * deltaTime;
	currentCamera.rot.y += (keyList[KEYBIND_LEFT].down - keyList[KEYBIND_RIGHT].down) * 1 * deltaTime;
	currentCamera.rot = (Vector3){fmod(currentCamera.rot.x, 6.28318), fmod(currentCamera.rot.y, 6.28318), fmod(currentCamera.rot.z, 6.28318)};
	currentCamera.focusDist = min(max(currentCamera.focusDist + (keyList[KEYBIND_I].down - keyList[KEYBIND_O].down) * 4 * max(1, sqrt(currentCamera.focusDist)) * deltaTime, 0), 64);
	
	//Vector3 invVec3 = {-1, -1, -1};
	//currentCamera.transform = genMatrix(vec3Mult(currentCamera.pos, invVec3), (Vector3){currentCamera.zoom, currentCamera.zoom, currentCamera.zoom}, vec3Mult(currentCamera.rot, invVec3));
	//currentCamera.transform = genMatrix((Vector3){0, 0, 0}, (Vector3){1, 1, 1}, currentCamera.rot);
	
	if(client.studio && focusObject)
		updateStudioGimbles();
	
	int idCounter = 0;
	objListLength = 0;
	if(!client.pause){
		//sunAngle = (Vector3){timer, timer, 0};
		//lightNormal = rotToNorm3(sunAngle);
		updateObjects(client.gameWorld->headObj, 0, &idCounter, false);
	}
	
	skyboxMatrix = translateMatrix(defaultMatrix, currentCamera.pos);
	drawMesh(skyboxMesh, skyboxMatrix, (SDL_FColor){1,1,1,1}, skyTex, false);
	free(skyboxMatrix);
	
	sunMatrix = genMatrix(currentCamera.pos, (Vector3){1, 1, 1}, sunAngle);
	drawMesh(sunMesh, sunMatrix, lightColour, sunTex, false);
	free(sunMatrix);
	
	//drawCube((Vector3){(2 + SDL_cos(timer)) / -2, SDL_sin(timer) + 1, (2 + SDL_cos(timer)) / -2}, (Vector3){2 + SDL_cos(timer), SDL_sin(timer) + 1, 2 + SDL_cos(timer)}, (SDL_FColor){0.6, 0.8, 1, 1});
	//drawCube((Vector3){SDL_sin(timer) * 2 - 0.5, 1, SDL_cos(timer) * 2 - 0.5}, (Vector3){1, 1, 1}, (SDL_FColor){1, 0.2, 0.3, 1});
	//SDL_SetRenderDrawColor(renderer, 255, 255, 255, SDL_ALPHA_OPAQUE);
	
	idCounter = 0;
	updateObjects(client.gameWorld->headObj, 0, &idCounter, true);
	
	updateStudio();
	
	updatePopups();
		
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
	drawText(renderer, &defaultFont, fpsText, 0, 0, 2, (SDL_FColor){1, 1, 1, 1});
	drawText(renderer, &defaultFont, rotText, 0, 16, 2, (SDL_FColor){1, 1, 1, 1});
	
	if(client.pause)drawText(renderer, &defaultFont, "Game Paused", 0, windowScale.y - 16, 2, (SDL_FColor){1, 1, 1, 1});
	
	/*if(!gameFileLoaded) {
		char* noGameText = "NO GAME HERE";
		int textWidth = strlen(noGameText) * 12;
		int centerX = (windowScale.x - textWidth) / 2;
		int centerY = windowScale.y / 2;
		drawText(renderer, fontTex, noGameText, 32, centerX, centerY, 16, 16, 12);
	}*/
	
	//drawText(renderer, fontTex, "Diagnostics: Skill issue", 32, 0, 64, 16, 16, 12);
	//SDL_RenderDebugText(renderer, 0, 0, guiText);

	if(glEnabled)
		updateOpenGL();
	free(currentCamera.transform);
	
	SDL_RenderPresent(renderer);

	return SDL_APP_CONTINUE;
}

void SDL_AppQuit(void *appstate, SDL_AppResult result){
	(void)appstate; (void)result;
	cleanupObjects(client.gameWorld->headObj);
	studioCleanup(); cleanupOpenGL();
	SDL_DestroyTexture(fontTex); SDL_DestroyTexture(playerTex); SDL_DestroyTexture(homerTex); SDL_DestroyTexture(cowTex); SDL_DestroyTexture(skyTex);
	
	free(defaultMatrix);
	free(playerMesh); free(skyboxMesh);
	free(planePrim); free(cubePrim); free(spherePrim);
}
    
void HandleKeyInput(){
	const bool* keyState = SDL_GetKeyboardState(NULL);
	bool hasFocus = SDL_GetWindowFlags(window) & SDL_WINDOW_INPUT_FOCUS;
	for(int i = 0; i < KEYBIND_MAX; i++){
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