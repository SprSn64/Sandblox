#define SDL_MAIN_USE_CALLBACKS 1
#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>
#include <SDL3_image/SDL_image.h>
#include <GL/glew.h>
//#include <iostream>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <dirent.h>

#include <structs.h>
#include "instances.h"
#include "renderer.h"
#include "math.h"
#include "loader.h"
#include "opengl.h"
#include "gamefile.h"
#include "bones.h"

#include "blockcode/blockcode.h"

#include "studio/studio.h"

#include "network/network.h"
#include "network/server.h"

#include "softwarerender/main.h"

/* TODO:
	Make studio widgets work properly (2/3 complete) and probably optimize them
	Add multiplayer server shit
	Implement simple physics
	Add secondary studio menu for changing object properties (colour, scale, etc)
*/

SDL_Window *window = NULL;
SDL_Renderer *renderer = NULL;

Sint32 glLocs[GLVAL_MAX];
Uint32 VAO, VBO, EBO;
Uint32 mainShader;
Uint32 glVersion[2] = {0, 0};
Uint32 glBlankTex; Uint32 blankColour = 0xFFFFFFFF;

extern TextureRef* studioTexRef;

FrameBuffer *gameBuffer;

TextureRef textBufferTex;

ClientData client;
GameWorld game;

//SDL_Point windowScaleIntent = {320, 240};
//double scaleFactor;
SDL_Point windowScale = {640, 480};
float aspectRatio = 1;
Texture* displayTex;
float* depthBuffer;

extern SDL_Rect objListRect;
extern float objListScroll;
extern Uint32 objListLength;

Camera currentCamera = {(Vector3){0, 2, 10}, (Vector3){0, 0, 0}, 90, 1, 16, NULL, NULL, NULL};
Uint8 camMoveMode = 0;
float mouseSense = 0.2;

bool mapLoaded = false;
bool gameFileLoaded = false;

Uint64 last = 0;
Uint64 now = 0;
double deltaTime = 0;
Uint32 lastFPS = 0;

float timer = 0;

Font defaultFont;

Texture *testTex;
Texture *rastFontTex;

TextureRef *fontTex = NULL; TextureRef *playerTex = NULL; TextureRef *homerTex = NULL;
TextureRef *boneTex = NULL; TextureRef *cursorTex = NULL; TextureRef *skyTex = NULL; TextureRef *sunTex = NULL;
Mesh *playerMesh = NULL; Mesh *playerFemMesh = NULL; Mesh *boneMesh = NULL; Mesh *skyboxMesh = NULL; Mesh *sunMesh = NULL;
Mesh *planePrim = NULL; Mesh *cubePrim = NULL; Mesh *spherePrim = NULL;
Mesh *frameBuffMesh = NULL;

ButtonMap keyList[KEYBIND_MAX];

SDL_MouseButtonFlags mouseState;
SDL_FPoint mousePos;
SDL_FPoint storedMousePos;
ButtonMap mouseButtons[3];
Uint8 camResetTimer = 0;

void HandleKeyInput();

extern DataObj gameHeader;
extern DataObj *focusObject;

extern Vector3 lightNormal;
extern SDL_FColor lightColour;
extern SDL_FColor lightAmbient;

DataObj* playerObj = NULL;
bool playerEnabled = true;

float* defaultMatrix = NULL;
float* skyboxMatrix = NULL; float* sunMatrix = NULL;
Vector3 sunAngle = {0, 0, 0};

float* guiMatrix = NULL;

char* clientPath;
char* basePath;

Server* debugServer = NULL;

cBlockClass testBlockClass = {0, (SDL_FColor){1, 0.83, 0.31, 1}, 0};
CodeBlock testCodeBlock;

//extern Uint64 memoryCount;

SDL_AppResult SDL_AppInit(void **appstate, int argc, char *argv[]){
	(void)appstate;
	client.version = "0.04 INDEV";
	SDL_SetAppMetadata("SandBlox", client.version, NULL);

	DIR* assetsDir = opendir("assets");
	if(!assetsDir){
		printf("Hey, where'd all my assets go?");
		return SDL_APP_FAILURE;
	}
	closedir(assetsDir);

	clientPath = SDL_GetCurrentDirectory();
	basePath = SDL_GetPrefPath("Sandblox", "Sandblox");

	char *mapToLoad = "assets/gamefile.json";

	for(int i=0; i < argc; i++){
		//printf("%s\n", argv[i]); 
		if(!strcmp("-debug", argv[i]))client.debug = true;
		if(!strcmp("-studio", argv[i]))client.studio = true; //currently broken!!!
		
		if(!strcmp("-mapfile", argv[i]))
			mapToLoad = argv[++i];
		if(!strcmp("-host", argv[i])) printf("cant host server at '%s'... not implemented yet sorry\n", argv[i+1]);
			//debugServer = serverInit(8080); //argv[i+1]
		if(!strcmp("-server", argv[i])) printf("cant join server '%s'... not implemented yet sorry\n", argv[i+1]);
			//connectServer(argv[i++]);
	}
	debugServer = serverInit(8080);
	client.server = debugServer;
	
	if(!SDL_Init(SDL_INIT_VIDEO)){
		SDL_Log("Couldn't initialize SDL: %s", SDL_GetError());
		return SDL_APP_FAILURE;
	}

	char windowName[64] = "Sandblox vXX.XX";
	sprintf(windowName, "Sandblox v%s", client.version);
	if(!(window = SDL_CreateWindow(windowName, windowScale.x, windowScale.y, SDL_WINDOW_RESIZABLE | SDL_WINDOW_OPENGL | SDL_WINDOW_MAXIMIZED))){
		printf("Couldn't create window: %s", SDL_GetError());
		return SDL_APP_FAILURE;
	}
	if(!SDL_GL_CreateContext(window)){
		printf("OpenGL initiation failed!\n");
	}
	glewInit();
	SDL_SetWindowMinimumSize(window, 320, 240);

	mainShader = loadShader("assets/shaders/default.vert", "assets/shaders/default.frag");
	
	glGenTextures(1, &glBlankTex);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, glBlankTex);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 1, 1, 0, GL_RGBA, GL_UNSIGNED_BYTE, &blankColour);
	glGenerateMipmap(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D, 0);

	SDL_GetWindowSize(window, &windowScale.x, &windowScale.y);
	gameBuffer = newFrameBuffer(windowScale.x, windowScale.y);

	fontTex = loadTexture("assets/textures/font.png", true);
	boneTex = loadTexture("assets/textures/bonetex.png", true);
	homerTex = loadTexture("assets/textures/homer.png", true);
	cursorTex = loadTexture("assets/textures/cursor.png", true);
	skyTex = loadTexture("assets/textures/skybox.png", true);
	sunTex = loadTexture("assets/textures/sunflare.png", true);

	rastFontTex = loadRasterTexture("assets/textures/font.png");
	defaultFont = (Font){fontTex, rastFontTex, 32, (SDL_Point){32, 32}, (SDL_Point){8, 8}, (SDL_FPoint){6, 0}, 16};

	playerMesh = loadMeshFromObj("assets/models/player.obj", true); 
	playerFemMesh = loadMeshFromObj("assets/models/playerfem.obj", true); 
	boneMesh = loadMeshFromObj("assets/models/bone.obj", true); 
	skyboxMesh = loadMeshFromObj("assets/models/advskybox.obj", true);
	sunMesh = loadMeshFromObj("assets/models/skyboxsun.obj", true);
	frameBuffMesh = loadMeshFromObj("assets/models/framebuffer.obj", true);
	
	planePrim = genPlaneMesh(1, 1, 1, 1); planePrim->persistent = true;
	cubePrim = loadMeshFromObj("assets/models/primitives/cube.obj", true);
	spherePrim = loadMeshFromObj("assets/models/primitives/sphere.obj", true);

	glGenVertexArrays(1, &VAO); glBindVertexArray(VAO);	
	glGenBuffers(1, &VBO); glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glGenBuffers(1, &EBO); glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
	
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 12 * sizeof(float), (void*)0); //pos
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 12 * sizeof(float), (void*)(3 * sizeof(float))); //norm
	glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 12 * sizeof(float), (void*)(6 * sizeof(float))); //uv
	glVertexAttribPointer(3, 4, GL_FLOAT, GL_FALSE, 12 * sizeof(float), (void*)(8 * sizeof(float))); //colour
	
	glEnableVertexAttribArray(0); 
	glEnableVertexAttribArray(1); 
	glEnableVertexAttribArray(2); 
	glEnableVertexAttribArray(3);
	
	//projMat = projMatrix(90, 4/3, 0.01, 10000);
	
	glLocs[GLVAL_WORLDMATRIX] = glGetUniformLocation(mainShader, "world");
	glLocs[GLVAL_VIEWMATRIX] = glGetUniformLocation(mainShader, "view");
	glLocs[GLVAL_PROJMATRIX] = glGetUniformLocation(mainShader, "proj");

	glLocs[GLVAL_LIGHTNORM] = glGetUniformLocation(mainShader, "lightNorm");
	glLocs[GLVAL_LIGHTCOLOUR] = glGetUniformLocation(mainShader, "lightColour");
	glLocs[GLVAL_AMBCOLOUR] = glGetUniformLocation(mainShader, "ambColour");
	glLocs[GLVAL_MULTCOLOUR] = glGetUniformLocation(mainShader, "multColour");

	glLocs[GLVAL_CAMERANORM] = glGetUniformLocation(mainShader, "cameraNorm");
	glLocs[GLVAL_RESOLUTION] = glGetUniformLocation(mainShader, "resolution");

	glLocs[GLVAL_TEXTURE0] = glGetUniformLocation(mainShader, "tex0");

	//glUniform1i(glLocs[GLVAL_TEXTURE0], 0);

	SDL_GL_SetSwapInterval(1);
	glEnable(GL_DEPTH_TEST); 
	glEnable(GL_CULL_FACE); glCullFace(GL_BACK); glFrontFace(GL_CCW);
	glClearColor(0, 0, 0, 1);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	glUniformMatrix4fv(glLocs[GLVAL_WORLDMATRIX], 1, GL_FALSE, defaultMatrix);

	keyList[KEYBIND_W].code = SDL_SCANCODE_W; keyList[KEYBIND_S].code = SDL_SCANCODE_S; keyList[KEYBIND_A].code = SDL_SCANCODE_A; keyList[KEYBIND_D].code = SDL_SCANCODE_D;
	keyList[KEYBIND_SPACE].code = SDL_SCANCODE_SPACE; keyList[KEYBIND_SHIFT].code = SDL_SCANCODE_LSHIFT;
	keyList[KEYBIND_UP].code = SDL_SCANCODE_UP; keyList[KEYBIND_DOWN].code = SDL_SCANCODE_DOWN; keyList[KEYBIND_LEFT].code = SDL_SCANCODE_LEFT; keyList[KEYBIND_RIGHT].code = SDL_SCANCODE_RIGHT;
	keyList[KEYBIND_I].code = SDL_SCANCODE_I; keyList[KEYBIND_O].code = SDL_SCANCODE_O; keyList[KEYBIND_Q].code = SDL_SCANCODE_Q; keyList[KEYBIND_E].code = SDL_SCANCODE_E;

	keyList[KEYBIND_SWAPRENDER].code = SDL_SCANCODE_TAB;
	keyList[KEYBIND_MENU].code = SDL_SCANCODE_ESCAPE;
	
	mouseButtons[0].code = SDL_BUTTON_LMASK; mouseButtons[1].code = SDL_BUTTON_MMASK; mouseButtons[2].code = SDL_BUTTON_RMASK;
	
	lightNormal = normalize3(lightNormal);
	
	initStudio();

	client.gameWorld = &game;
	client.gameWorld->headObj = &gameHeader;
	gameHeader.studioOpen = true;

	client.gameWorld->currPlayer = NULL;
	client.gameWorld->currCamera = &currentCamera;
	client.gameWorld->playerRespawn = 5;
	client.gameWorld->skybox = NULL;

	client.debug = true;
	
	defaultMatrix = newMatrix();

	testCodeBlock = (CodeBlock){&testBlockClass, (SDL_FPoint){24, 24}, NULL, NULL, NULL, NULL};

	//SDL_HideCursor();
	
	//if(glEnabled) goto openGlInitSkip;

//openGlInitSkip:

	if(mapLoaded) return SDL_APP_CONTINUE;
	
	if(mapToLoad && loadGameFile(mapToLoad) == 0){
		gameFileLoaded = true;
	} else {
		printf("Failed to load gamefile\n");
		sendPopup("Failed to load gamefile", NULL, NULL, 3);
		gameFileLoaded = false;
		client.pause = true;
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
		bool studioFocus = false;//studioWindow && (SDL_GetWindowFlags(studioWindow) & SDL_WINDOW_INPUT_FOCUS);
		
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

extern void studioCameraUpdate(Camera* cam);
extern StudioSplit panelHead;

SDL_AppResult SDL_AppIterate(void *appstate){
	(void)appstate;
	HandleKeyInput();
	
	mouseState = SDL_GetMouseState(&mousePos.x, &mousePos.y);
	for(int i=0; i<3; i++){
		mouseButtons[i].down = (SDL_GetWindowFlags(window) & SDL_WINDOW_INPUT_FOCUS) && (mouseState & mouseButtons[i].code);
		mouseButtons[i].pressed = false;
		if(mouseButtons[i].down){
			if(!mouseButtons[i].pressCheck){
				mouseButtons[i].pressCheck = true;
				mouseButtons[i].pressed = true;
			}
		}else mouseButtons[i].pressCheck = false;
	}
	
	last = now;
	now = SDL_GetTicks();
	deltaTime = min(((double)now - (double)last) / 1000.0f, 1);
	timer += deltaTime;
	
	SDL_GetWindowSize(window, &windowScale.x, &windowScale.y);
	aspectRatio = (float)windowScale.x/windowScale.y;

	guiMatrix = isoProjMatrix(1, aspectRatio, 0.01, 1000);

	if(debugServer)
		serverUpdate();
	
	//SDL_ShowCursor();
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
			float moveScale = mouseSense;// / (((float)windowScale.x + windowScale.y) / 1280);
			currentCamera.rot.x += -(floor(mousePos.y) - floor(storedMousePos.y)) * moveScale * deltaTime;
			currentCamera.rot.y += -(floor(mousePos.x) - floor(storedMousePos.x)) * moveScale * deltaTime;  
			camResetTimer++;
			if(camResetTimer >= 4){
				SDL_WarpMouseInWindow(window, storedMousePos.x, storedMousePos.y); 
				camResetTimer = 0;
			}
			//SDL_HideCursor();
		}else{
			storedMousePos = mousePos;
			camMoveMode = 1;
		}
	}else camMoveMode = 0;

	if(keyList[KEYBIND_SWAPRENDER].pressed){
		sendPopup("fuck", NULL, NULL, 3);
	}

	if(keyList[KEYBIND_MENU].pressed){
		client.pause = !client.pause;
	}
	
	currentCamera.rot.x += (keyList[KEYBIND_UP].down - keyList[KEYBIND_DOWN].down) * 1 * deltaTime;
	currentCamera.rot.y += (keyList[KEYBIND_LEFT].down - keyList[KEYBIND_RIGHT].down) * 1 * deltaTime;
	currentCamera.rot = (Vector3){fmod(currentCamera.rot.x, 6.28318), fmod(currentCamera.rot.y, 6.28318), fmod(currentCamera.rot.z, 6.28318)};
	currentCamera.focusDist = min(max(currentCamera.focusDist + (keyList[KEYBIND_I].down - keyList[KEYBIND_O].down) * 4 * max(1, sqrt(currentCamera.focusDist)) * deltaTime, 0), 64);

	int idCounter = 0;
	objListLength = 0;
	if(!client.pause){
		if(playerEnabled && !client.gameWorld->currPlayer){
			if(client.gameWorld->playerRespawn >= 3) loadPlayerAvatar();
			client.gameWorld->playerRespawn += deltaTime;
		}
		updateObjects(client.gameWorld->headObj, 0, &idCounter);
	}

	glViewport(0, 0, windowScale.x, windowScale.y);

	//freeFrameBuffer(gameBuffer);
	//gameBuffer = newFrameBuffer(windowScale.x, windowScale.y);
	bindFrameBuffer(gameBuffer);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	setGlShader(mainShader);

	if(client.pause)
		studioCameraUpdate(client.gameWorld->currCamera);

	Vector3 invVec3 = {-1, -1, -1};
	client.gameWorld->currCamera->transform = malloc(sizeof(mat4));
	
	float *camTranslated = translateMatrix(defaultMatrix, vec3Mult(client.gameWorld->currCamera->pos, invVec3));
	float *camRotated = rotateMatrix(camTranslated, vec3Mult(client.gameWorld->currCamera->rot, invVec3), ROT_YXZ);
	
	memcpy(client.gameWorld->currCamera->transform, camRotated, sizeof(mat4));
	free(camTranslated); free(camRotated); 

	client.gameWorld->currCamera->proj = projMatrix(currentCamera.fov, aspectRatio, 0.1, 1000000);

	if(client.studio && focusObject)
		updateStudioGimbles();

	glUniformMatrix4fv(glLocs[GLVAL_PROJMATRIX], 1, GL_FALSE, currentCamera.proj);
	glUniformMatrix4fv(glLocs[GLVAL_VIEWMATRIX], 1, GL_FALSE, currentCamera.transform);

	float resFloat[2] = {0, 0};
	glUniform2fv(glLocs[GLVAL_RESOLUTION], 1, resFloat);

	glUniform3fv(glLocs[GLVAL_LIGHTNORM], 1, (float*)&lightNormal);
	Vector3 cameraNormal = rotToNorm3(client.gameWorld->currCamera->rot);
	glUniform3fv(glLocs[GLVAL_CAMERANORM], 1, (float*)&cameraNormal);

	SDL_FColor flatAmb = {1, 1, 1, 1}; SDL_FColor flatLight = {0, 0, 0, 1};

	setGlValue(GL_DEPTH_TEST, false); setGlValue(GL_BLEND, true);
	glUniform4fv(glLocs[GLVAL_LIGHTCOLOUR], 1, (float*)&flatLight);
	glUniform4fv(glLocs[GLVAL_AMBCOLOUR], 1, (float*)&flatAmb);

		skyboxMatrix = translateMatrix(defaultMatrix, currentCamera.pos);
		TextureRef* skyboxTex = skyTex;
		if(client.gameWorld->skybox)
			skyboxTex = client.gameWorld->skybox;
		drawMeshOpenGL(skyboxMesh, skyboxMatrix, (SDL_FColor){1,1,1,1}, skyboxTex);
		free(skyboxMatrix);
	
		sunMatrix = genMatrix(currentCamera.pos, (Vector3){1, 1, 1}, vec3Add(normToRot3(lightNormal), (Vector3){PI, PI, 0}));
		drawMeshOpenGL(sunMesh, sunMatrix, lightColour, sunTex);
		free(sunMatrix);

	setGlValue(GL_DEPTH_TEST, true); setGlValue(GL_BLEND, false);

	resFloat[0] = windowScale.x; resFloat[1] = windowScale.y;
	glUniform2fv(glLocs[GLVAL_RESOLUTION], 1, resFloat);

	Uint32 glError = glGetError();
	if(glError != GL_NO_ERROR)
		printf("GL ERROR: %d\n", glError);
	
	//drawCube((Vector3){(2 + SDL_cos(timer)) / -2, SDL_sin(timer) + 1, (2 + SDL_cos(timer)) / -2}, (Vector3){2 + SDL_cos(timer), SDL_sin(timer) + 1, 2 + SDL_cos(timer)}, (SDL_FColor){0.6, 0.8, 1, 1});
	//drawCube((Vector3){SDL_sin(timer) * 2 - 0.5, 1, SDL_cos(timer) * 2 - 0.5}, (Vector3){1, 1, 1}, (SDL_FColor){1, 0.2, 0.3, 1});
	//SDL_SetRenderDrawColor(renderer, 255, 255, 255, SDL_ALPHA_OPAQUE);

	glUniform4fv(glLocs[GLVAL_LIGHTCOLOUR], 1, (float*)&lightColour);
	glUniform4fv(glLocs[GLVAL_AMBCOLOUR], 1, (float*)&lightAmbient);
	
	idCounter = 0;
	drawObjects(client.gameWorld->headObj, 0, &idCounter);

	//drawSkip:

	updateStudio();
		
	static char fpsText[256] = "FPS: 0";
	static char rotText[256] = "Camera Rot: 0, 0";
	static double lastDebugUpdate = 0;
	if ((Uint32)(timer*100)%64 == 0)
		lastFPS = (Uint32)floor(1/deltaTime);
	if(timer - lastDebugUpdate >= 0.5){
		lastDebugUpdate = timer;
		sprintf(fpsText, "FPS: %d", lastFPS);
		sprintf(rotText, "Camera Rot: %d, %d", (int)(currentCamera.rot.y * RAD2DEG), (int)(currentCamera.rot.x * RAD2DEG));
	}
	drawText(renderer, &defaultFont, fpsText, 0, 0, 2, (SDL_FColor){1, 1, 1, 1});
	drawText(renderer, &defaultFont, rotText, 0, 16, 2, (SDL_FColor){1, 1, 1, 1});
	
	if(client.pause)drawText(renderer, &defaultFont, "Game Paused", 0, windowScale.y - 16, 2, (SDL_FColor){1, 1, 1, 1});

	setGlValue(GL_DEPTH_TEST, false); setGlValue(GL_BLEND, true);
	glUniformMatrix4fv(glLocs[GLVAL_PROJMATRIX], 1, GL_FALSE, guiMatrix);
	glUniformMatrix4fv(glLocs[GLVAL_VIEWMATRIX], 1, GL_FALSE, defaultMatrix);
	resFloat[0] = 0; resFloat[1] = 0;
	glUniform2fv(glLocs[GLVAL_RESOLUTION], 1, resFloat);
	glUniform4fv(glLocs[GLVAL_LIGHTCOLOUR], 1, (float*)&flatLight);
	glUniform4fv(glLocs[GLVAL_AMBCOLOUR], 1, (float*)&flatAmb);

	updatePopups();

	if(SDL_GetWindowFlags(window) & SDL_WINDOW_MOUSE_FOCUS){
		SDL_FPoint cursorDrawPos = camMoveMode == 1 ? storedMousePos : mousePos;
		if(camMoveMode == 2) cursorDrawPos = (SDL_FPoint){windowScale.x >> 1, windowScale.y >> 1};
		float* cursorMatrix = genMatrix(
			screenToGL((Vector3){floor(cursorDrawPos.x), floor(cursorDrawPos.y), 0}), 
			(Vector3){(32.f / windowScale.x) * aspectRatio * 2, 1, (32.f / windowScale.y) * 2}, 
			(Vector3){HALFPI, 0, 0});
		drawMeshOpenGL(planePrim, cursorMatrix, (SDL_FColor){1, 1, 1, 1}, cursorTex);
		free(cursorMatrix);
	}
	bindFrameBuffer(NULL);

	float* gameMatrix = genMatrix((Vector3){0, 0, 0}, (Vector3){aspectRatio, 1, 1}, (Vector3){0, 0, 0});
	drawMeshOpenGL(frameBuffMesh, gameMatrix, (SDL_FColor){1, 1, 1, 1}, gameBuffer->texture);
	free(gameMatrix);

	if(client.studio){
		float* studioMatrix = genMatrix((Vector3){-aspectRatio, 1, 0}, (Vector3){(240.f / windowScale.x) * aspectRatio * 2, 1, (320.f / windowScale.y) * 2}, (Vector3){HALFPI, 0, 0});
		drawMeshOpenGL(planePrim, studioMatrix, (SDL_FColor){1, 1, 1, 1}, studioTexRef);
		free(studioMatrix);
	}
	drawSplit(&panelHead);

	free(guiMatrix); setGlValue(GL_BLEND, false);

	free(currentCamera.transform);
	free(currentCamera.proj);
	
	SDL_GL_SwapWindow(window);

	return SDL_APP_CONTINUE;
}

void SDL_AppQuit(void *appstate, SDL_AppResult result){
	(void)appstate; (void)result;
	cleanupObjects(client.gameWorld->headObj);
	studioCleanup();
	cleanupTextures(false); cleanupMeshes(false);

	free(defaultMatrix);
	//free(depthBuffer); freeRasterTexture(displayTex); freeRasterTexture(rastFontTex);

	glDeleteProgram(mainShader);
	glDeleteBuffers(1, &VAO); glDeleteBuffers(1, &VBO); glDeleteBuffers(1, &EBO);
	glDeleteTextures(1, &glBlankTex); freeFrameBuffer(gameBuffer);
}

void HandleKeyInput(){
	const bool* keyState = SDL_GetKeyboardState(NULL);
	bool hasFocus = SDL_GetWindowFlags(window) & SDL_WINDOW_INPUT_FOCUS;
	for(int i = 0; i < KEYBIND_MAX; i++){
		keyList[i].down = keyState[keyList[i].code] && hasFocus;
		keyList[i].pressed = false;
		if(keyList[i].down){
			if(!keyList[i].pressCheck){
				keyList[i].pressCheck = true;
				keyList[i].pressed = true;
			}
		}else keyList[i].pressCheck = false;
	}
}