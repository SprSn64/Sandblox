#include "studio.h"
#include "structs.h"

#include <SDL3/SDL.h>
#include <SDL3_image/SDL_image.h>
#include <GL/glew.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#include "gamefile.h"
#include "../math.h"
#include "../entities.h"
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
TextureRef *bannerButtonTex = NULL;

Uint32 toolMode = STUDIOTOOL_MOVE;

DataObj *focusObject = NULL;
extern SDL_MouseButtonFlags mouseState;
extern SDL_FPoint mousePos;
extern ButtonMap mouseButtons[3];
extern ButtonMap keyList[KEYBIND_MAX];

extern Mesh *spherePrim;
extern Mesh *planePrim;
extern TextureRef *cursorTex;

extern TextureRef *textBufferTex;

StudioPanel testGamePanel = {STUDIOPANEL_GAMEVIEW, NULL, false, NULL};
StudioPanel testToolPanel = {STUDIOPANEL_TOOLBAR, NULL, false, NULL};
StudioPanel toolGameSplit = {STUDIOPANEL_SPLIT, NULL, false, NULL};

StudioPanel testExplorerPanel = {STUDIOPANEL_EXPLORER, NULL, false, NULL};
StudioPanel gameExplorerSplit = {STUDIOPANEL_SPLIT, NULL, false, NULL};

StudioPanel bannerSplitPanel = {STUDIOPANEL_SPLIT, NULL, false, NULL};
StudioPanel bannarPanel = {STUDIOPANEL_BANNER, NULL, false, NULL};

void drawTextureSlice(TextureRef* buffer, TextureRef* tex, SDL_Rect src, SDL_Rect dest){
	bufferGlSlice(buffer, tex, src);

	float* destMatrix = genMatrix(
		(Vector3){((float)dest.x / (windowScale.x >> 1) - 1) * aspectRatio, -(float)(dest.y) / (windowScale.y >> 1) + 1, 0}, 
		(Vector3){(float)dest.w / (windowScale.x >> 1) * aspectRatio, 0, (float)dest.h / (windowScale.y >> 1)}, 
		(Vector3){HALFPI, 0, 0}
	);
	drawMeshOpenGL(planePrim, destMatrix, (SDL_FColor){1, 1, 1, 1}, buffer);
	free(destMatrix);
}

void initPanel(StudioPanel* panel);
void updatePanel(StudioPanel* panel, SDL_Rect area);
void drawPanel(StudioPanel* panel, SDL_Rect area);

void parentPanel(StudioPanel* panel, StudioPanel* split, bool target){
	if(split->type != STUDIOPANEL_SPLIT) return;

	panel->parent = split;
	SplitPanelHandler* handler = split->handler;

	if(target)
		handler->childB = panel;
	else
		handler->childA = panel;
}

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

	toolButtonTex = loadTexture("assets/textures/studio/studiobuttons.png", true);
	bannerButtonTex = loadTexture("assets/textures/studio/bannerbuttons.png", true);

	initPanel(&toolGameSplit); initPanel(&testGamePanel); initPanel(&testToolPanel);
	initPanel(&bannerSplitPanel); initPanel(&bannarPanel); 

	initPanel(&testExplorerPanel); initPanel(&gameExplorerSplit);

	parentPanel(&bannarPanel, &bannerSplitPanel, false);
	parentPanel(&testToolPanel, &toolGameSplit, false);
	parentPanel(&testGamePanel, &toolGameSplit, true);

	parentPanel(&toolGameSplit, &gameExplorerSplit, false);
	parentPanel(&testExplorerPanel, &gameExplorerSplit, true);

	parentPanel(&gameExplorerSplit, &bannerSplitPanel, true);

	SplitPanelHandler* testSplitHandler = toolGameSplit.handler;
	testSplitHandler->ratio = 0.05;

	SplitPanelHandler* explorerSplitHandler = gameExplorerSplit.handler;
	explorerSplitHandler->ratio = 0.8;

	SplitPanelHandler* bannerSplitHandler = bannerSplitPanel.handler;
	bannerSplitHandler->ratio = 0.1; bannerSplitHandler->vert = true;
}

void updateStudio(){
	updatePanel(&bannerSplitPanel, (SDL_Rect){0, 0, windowScale.x, windowScale.y});
}

void renderStudio(){
	drawPanel(&bannerSplitPanel, (SDL_Rect){0, 0, windowScale.x, windowScale.y});

	/*if(SDL_GetWindowFlags(window) & SDL_WINDOW_MOUSE_FOCUS){
		float* cursorMatrix = genMatrix(
			screenToGL((Vector3){floor(mousePos.x), floor(mousePos.y), 0}), 
			(Vector3){(32.f / windowScale.x) * aspectRatio * 2, 1, (32.f / windowScale.y) * 2}, 
			(Vector3){HALFPI, 0, 0});
		drawMeshOpenGL(planePrim, cursorMatrix, (SDL_FColor){1, 1, 1, 1}, cursorTex);
		free(cursorMatrix);
	}*/
}

void freeStudio(){
	if(!studioActive) return;
}

float camTime = 0;
void studioCameraUpdate(Camera* camera){
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
	
	float* camRotMatrix = rotateMatrix(defaultMatrix, camera->rot, ROT_XYZ);
	moveVec = matrixMult(moveVec, camRotMatrix);
	
	camera->pos = (Vector3){camera->pos.x + moveVec.x * timeSpeedMult * deltaTime, camera->pos.y + moveVec.y * timeSpeedMult * deltaTime, camera->pos.z + moveVec.z * timeSpeedMult * deltaTime};
	free(camRotMatrix);
}

void splitPanelInit(StudioPanel* panel){
	SplitPanelHandler* handler = malloc(sizeof(SplitPanelHandler));
	if(!handler) return;

	handler->childA = NULL; handler->childB = NULL;
	handler->ratio = 0.5; handler->vert = false; 
	panel->handler = handler;
}
void splitPanelUpdate(StudioPanel* panel, SDL_Rect area){
	SplitPanelHandler* handler = panel->handler;
	if(!handler) return;

	SDL_Rect areaA;
	SDL_Rect areaB;
	if(handler->vert){
		areaA = (SDL_Rect){area.x, area.y, area.w, area.h * handler->ratio};
		areaB = (SDL_Rect){area.x, area.y + area.h * handler->ratio, area.w, area.h * (1 - handler->ratio)};
	}else{
		areaA = (SDL_Rect){area.x, area.y, area.w * handler->ratio, area.h};
		areaB = (SDL_Rect){area.x + area.w * handler->ratio, area.y, area.w * (1 - handler->ratio), area.h};
	}

	if(handler->childA)
		updatePanel(handler->childA, areaA);
	if(handler->childB)
		updatePanel(handler->childB, areaB);
}
void splitPanelDraw(StudioPanel* panel, SDL_Rect area){
	SplitPanelHandler* handler = panel->handler;
	if(!handler) return;

	SDL_Rect areaA;
	SDL_Rect areaB;
	if(handler->vert){
		areaA = (SDL_Rect){area.x, area.y, ceil(area.w), ceil(area.h * handler->ratio)};
		areaB = (SDL_Rect){area.x, area.y + ceil(area.h * handler->ratio), ceil(area.w), ceil(area.h * (1 - handler->ratio))};
	}else{
		areaA = (SDL_Rect){area.x, area.y, ceil(area.w * handler->ratio), ceil(area.h)};
		areaB = (SDL_Rect){area.x + ceil(area.w * handler->ratio), area.y, ceil(area.w * (1 - handler->ratio)), ceil(area.h)};
	}

	float* aMatrix = genMatrix(
		(Vector3){((float)areaA.x / (windowScale.x >> 1) - 1) * aspectRatio, (float)(-areaA.y) / (windowScale.y >> 1) + 1, 0}, 
		(Vector3){(float)areaA.w / (windowScale.x >> 1) * aspectRatio, 0, (float)areaA.h / (windowScale.y >> 1)}, 
		(Vector3){HALFPI, 0, 0}
	);
	drawMeshOpenGL(planePrim, aMatrix, (SDL_FColor){1, 0, 0, 0.5}, NULL);
	free(aMatrix);

	float* bMatrix = genMatrix(
		(Vector3){((float)areaB.x / (windowScale.x >> 1) - 1) * aspectRatio, (float)(-areaB.y) / (windowScale.y >> 1) + 1, 0}, 
		(Vector3){(float)areaB.w / (windowScale.x >> 1) * aspectRatio, 0, (float)areaB.h / (windowScale.y >> 1)}, 
		(Vector3){HALFPI, 0, 0}
	);
	drawMeshOpenGL(planePrim, bMatrix, (SDL_FColor){0, 0, 1, 0.5}, NULL);
	free(bMatrix);

	if(handler->childA)
		drawPanel(handler->childA, areaA);
	if(handler->childB)
		drawPanel(handler->childB, areaB);
}

void gamePanelInit(StudioPanel* panel){
	GamePanelHandler* handler = malloc(sizeof(GamePanelHandler));
	if(!handler) return;

	handler->camera = client.gameWorld->currCamera;
	handler->buffer = newFrameBuffer(1920, 1080);
	panel->handler = handler;
}
extern SDL_Rect viewArea; 
void gamePanelUpdate(StudioPanel* panel, SDL_Rect area){
	if(!panel->mouseOver) return;
	viewArea = area;

	if(focusObject) updateStudioGimbles();
}

extern Mesh* frameBuffMesh;
void renderGame(FrameBuffer* target, Camera* cam, bool renderGui);
void gamePanelDraw(StudioPanel* panel, SDL_Rect area){
	GamePanelHandler* handler = panel->handler;
	if(!handler) return;
	if(!handler->buffer || !handler->camera) return;

	Camera* targetCamera = handler->camera;
	if(focusObject && focusObject->classData == &cameraClass){
		targetCamera = focusObject->objOther;
	}

	freeFrameBuffer(handler->buffer);
	handler->buffer = newFrameBuffer(area.w, area.h);

	renderGame(handler->buffer, targetCamera, true);
	bindFrameBuffer(NULL); glViewport(0, 0, windowScale.x, windowScale.y);
	
	float* gameMatrix = genMatrix(
		(Vector3){((float)(area.x + (area.w >> 1)) / (windowScale.x >> 1) - 1) * aspectRatio, -(float)(area.y + (area.h >> 1)) / (windowScale.y >> 1) + 1, 0}, 
		(Vector3){(float)(area.w >> 1) / (windowScale.x >> 1) * aspectRatio, (float)(area.h >> 1) / (windowScale.y >> 1), 0}, 
		(Vector3){0, 0, 0}
	);
	drawMeshOpenGL(frameBuffMesh, gameMatrix, (SDL_FColor){1, 1, 1, 1}, handler->buffer->texture);
	free(gameMatrix);
}

void toolbarPanelInit(StudioPanel* panel){
	(void)panel;
}
void toolbarPanelUpdate(StudioPanel* panel, SDL_Rect area){
	if(!panel->mouseOver) return;
	SDL_Point localMouse = {mousePos.x - area.x, mousePos.y - area.y};

	if(mouseButtons[0].pressed)
		toolMode = min((int)((float)localMouse.y / 64), STUDIOTOOL_ROTATE);
}
void toolbarPanelDraw(StudioPanel* panel, SDL_Rect area){
	(void)panel;
	//SplitPanelHandler* handler = panel->handler;
	//if(!handler) return;

	float* bgMatrix = genMatrix(
		(Vector3){((float)area.x / (windowScale.x >> 1) - 1) * aspectRatio, (float)(-area.y) / (windowScale.y >> 1) + 1, 0}, 
		(Vector3){(float)area.w / (windowScale.x >> 1) * aspectRatio, 0, (float)area.h / (windowScale.y >> 1)}, 
		(Vector3){HALFPI, 0, 0}
	);
	drawMeshOpenGL(planePrim, bgMatrix, (SDL_FColor){0.4, 0.4, 0.4, 1}, NULL);
	free(bgMatrix);

	drawTextureSlice(textBufferTex, toolButtonTex, (SDL_Rect){16, 16, 16, 16}, (SDL_Rect){area.x, area.y, 16, 16});
}

void bannerPanelInit(StudioPanel* panel){
	(void)panel; 
}
static void SDLCALL loadMapDialogue(void* userdata, const char* const* filelist, int filter);
static void SDLCALL saveMapDialogue(void* userdata, const char* const* filelist, int filter);
static const SDL_DialogFileFilter mapLoadFilter[] = {{"JSON Map", "json"}, {"All File Types", "*"}};
void bannerPanelUpdate(StudioPanel* panel, SDL_Rect area){
	StudioPanel* split = panel->parent;
	if(!split || split->type != STUDIOPANEL_SPLIT) return;

	SplitPanelHandler* splitHandler = split->handler;

	if(!splitHandler->vert) return;
	bool splitHalf = splitHandler->childB == panel ? true : false;

	float splitArea = area.h / (splitHalf ? (1 - splitHandler->ratio) : splitHandler->ratio);
	splitHandler->ratio = 64 / splitArea;

	if(!panel->mouseOver) return;

	if(mouseButtons[0].pressed && between(mousePos.x - (area.x + 16), 0, 32) && between(mousePos.y - (area.y + 16), 0, 32)){
		SDL_ShowSaveFileDialog(saveMapDialogue, NULL, window, mapLoadFilter, SDL_arraysize(mapLoadFilter), SDL_GetCurrentDirectory());
	}
	if(mouseButtons[0].pressed && between(mousePos.x - (area.x + 50), 0, 32) && between(mousePos.y - (area.y + 16), 0, 32)){
		SDL_ShowOpenFileDialog(loadMapDialogue, NULL, window, mapLoadFilter, SDL_arraysize(mapLoadFilter), SDL_GetCurrentDirectory(), false);
	}
}
void bannerPanelDraw(StudioPanel* panel, SDL_Rect area){
	(void)panel; 
	float* bgMatrix = genMatrix(
		(Vector3){((float)area.x / (windowScale.x / 2) - 1) * aspectRatio, -(float)(area.y) / (windowScale.y / 2) + 1, 0}, 
		(Vector3){(float)area.w / (windowScale.x / 2) * aspectRatio, 0, (float)area.h / (windowScale.y / 2)}, 
		(Vector3){HALFPI, 0, 0}
	);
	drawMeshOpenGL(planePrim, bgMatrix, (SDL_FColor){0.4, 0.4, 0.4, 1}, NULL);
	free(bgMatrix);

	drawTextureSlice(textBufferTex, bannerButtonTex, (SDL_Rect){32, 0, 32, 32}, (SDL_Rect){area.x + 16, area.y + 16, 32, 32});
	drawTextureSlice(textBufferTex, bannerButtonTex, (SDL_Rect){64, 0, 32, 32}, (SDL_Rect){area.x + 50, area.y + 16, 32, 32});
}

float explTextSize = 16;
void explorerPanelInit(StudioPanel* panel){
	(void)panel; 
}
void updateExplorerItem(DataObj* item, SDL_Rect area, int *idCount){
	int i = (*idCount)++;

	if(mouseButtons[0].pressed && between(mousePos.y - area.y, i * explTextSize, (i + 1) * explTextSize)){
		if(focusObject == item)
			item->studioOpen = !item->studioOpen;

		focusObject = item;
	}

	if(!item->studioOpen) return;
	DataObj* child = item->child;
	while(child){
		DataObj *next = child->next;
		updateExplorerItem(child, area, idCount);
		child = next;
	}
}
void explorerPanelUpdate(StudioPanel* panel, SDL_Rect area){
	(void)panel;

	if(!panel->mouseOver) return;

	int listY = 0;
	updateExplorerItem(client.gameWorld->headObj, area, &listY);
}
void drawExplorerItem(DataObj* item, SDL_Rect area, int nodeDepth, int *idCount){
	int i = (*idCount)++;

	if(item == focusObject){
		float* backMatrix = genMatrix(
			(Vector3){((float)(area.x) / (windowScale.x / 2) - 1) * aspectRatio, -(float)(area.y + explTextSize * i) / (windowScale.y / 2) + 1, 0}, 
			(Vector3){(float)(area.w) / (windowScale.x / 2) * aspectRatio, 0, explTextSize / (windowScale.y / 2)}, 
			(Vector3){HALFPI, 0, 0}
		);
		drawMeshOpenGL(planePrim, backMatrix, (SDL_FColor){0.25, 0.75, 0.18625, 1}, NULL);
		free(backMatrix);
	}

	bufferGlSlice(textBufferTex, classIconTex, (SDL_Rect){(item->classData->id % 16) * 16, (int)floor((float)item->classData->id / 16) * 16 % 256, 16, 16});
	float* iconMatrix = genMatrix(
		(Vector3){((float)(area.x + 24 * nodeDepth) / (windowScale.x / 2) - 1) * aspectRatio, -(float)(area.y + explTextSize * i) / (windowScale.y / 2) + 1, 0}, 
		(Vector3){(float)(16) / (windowScale.x / 2) * aspectRatio, 0, (float)(16) / (windowScale.y / 2)}, 
		(Vector3){HALFPI, 0, 0}
	);
	drawMeshOpenGL(planePrim, iconMatrix, (SDL_FColor){1, 1, 1, 1}, textBufferTex);
	free(iconMatrix);

	if(!item->studioOpen && item->child){
		bufferGlSlice(textBufferTex, classIconTex, (SDL_Rect){245, 249, 11, 7});
		float* plusMatrix = genMatrix(
			(Vector3){((float)(area.x + 24 * nodeDepth) / (windowScale.x / 2) - 1) * aspectRatio, -(float)(area.y + explTextSize * i) / (windowScale.y / 2) + 1, 0}, 
			(Vector3){11.f / (windowScale.x / 2) * aspectRatio, 0, 7.f / (windowScale.y / 2)}, 
			(Vector3){HALFPI, 0, 0}
		);
		drawMeshOpenGL(planePrim, plusMatrix, (SDL_FColor){1, 1, 1, 1}, textBufferTex);
		free(plusMatrix);
	}

	float textRatio = bufferGLText(textBufferTex, &defaultFont, item->name, 2);
	float* textMatrix = genMatrix(
		(Vector3){((float)(24 + area.x + 24 * nodeDepth) / (windowScale.x / 2) - 1) * aspectRatio, -(float)(area.y + explTextSize * i) / (windowScale.y / 2) + 1, 0}, 
		(Vector3){(float)(explTextSize / textRatio) / (windowScale.x / 2) * aspectRatio, 0, explTextSize / (windowScale.y / 2)}, 
		(Vector3){HALFPI, 0, 0}
	);
	drawMeshOpenGL(planePrim, textMatrix, (SDL_FColor){1, 1, 1, 1}, textBufferTex);
	free(textMatrix);

	if(!item->studioOpen) return;
	DataObj* child = item->child;
	while(child){
		DataObj *next = child->next;
		drawExplorerItem(child, area, nodeDepth + 1, idCount);
		child = next;
	}
}
void explorerPanelDraw(StudioPanel* panel, SDL_Rect area){
	(void)panel; 
	float* bgMatrix = genMatrix(
		(Vector3){((float)area.x / (windowScale.x / 2) - 1) * aspectRatio, (float)(-area.y) / (windowScale.y / 2) + 1, 0}, 
		(Vector3){(float)area.w / (windowScale.x / 2) * aspectRatio, 0, (float)area.h / (windowScale.y / 2)}, 
		(Vector3){HALFPI, 0, 0}
	);
	drawMeshOpenGL(planePrim, bgMatrix, (SDL_FColor){0.4, 0.4, 0.4, 1}, NULL);
	free(bgMatrix);

	int listY = 0;
	drawExplorerItem(client.gameWorld->headObj, area, 0, &listY);
}

void initPanel(StudioPanel* panel){
	switch(panel->type){
		case STUDIOPANEL_GAMEVIEW: gamePanelInit(panel); return;
		case STUDIOPANEL_SPLIT: splitPanelInit(panel); return;
		case STUDIOPANEL_TOOLBAR: toolbarPanelInit(panel); return;
		default: break;
	}
}

void updatePanel(StudioPanel* panel, SDL_Rect area){
	panel->mouseOver = (SDL_GetWindowFlags(window) & SDL_WINDOW_MOUSE_FOCUS) ? between(mousePos.x - area.x, 0, area.w) && between(mousePos.y - area.y, 0, area.h) : false;

	switch(panel->type){
		case STUDIOPANEL_GAMEVIEW: gamePanelUpdate(panel, area); return;
		case STUDIOPANEL_SPLIT: splitPanelUpdate(panel, area); return;
		case STUDIOPANEL_TOOLBAR: toolbarPanelUpdate(panel, area); return;
		case STUDIOPANEL_BANNER: bannerPanelUpdate(panel, area); return;
		case STUDIOPANEL_EXPLORER: explorerPanelUpdate(panel, area); return;
		default: break;
	}
}

void drawPanel(StudioPanel* panel, SDL_Rect area){
	switch(panel->type){
		case STUDIOPANEL_GAMEVIEW: gamePanelDraw(panel, area); return;
		case STUDIOPANEL_SPLIT: splitPanelDraw(panel, area); return;
		case STUDIOPANEL_TOOLBAR: toolbarPanelDraw(panel, area); return;
		case STUDIOPANEL_BANNER: bannerPanelDraw(panel, area); return;
		case STUDIOPANEL_EXPLORER: explorerPanelDraw(panel, area); return;
		default: break;
	}
}

static void SDLCALL loadMapDialogue(void* userdata, const char* const* filelist, int filter){
	(void)userdata; (void)filter;

	if(!filelist || !*filelist) return;

	client.gameWorld->headObj->child = NULL;
	focusObject = NULL;
	
	loadGameFile(*filelist);
};
static void SDLCALL saveMapDialogue(void* userdata, const char* const* filelist, int filter){
	(void)userdata; (void)filter;
	if(!filelist || !*filelist) return;

	saveGameFile(*filelist);
}