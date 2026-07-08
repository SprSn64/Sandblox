#include "studio.h"
#include <structs.h>

#include <SDL3/SDL.h>
#include <SDL3_image/SDL_image.h>
#include <GL/glew.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#include "../opengl.h"
#include "../math.h"
#include "../renderer.h"
#include "../instances.h"

extern float aspectRatio;
extern float* defaultMatrix;
extern SDL_Point windowScale;
extern Mesh *planePrim;

extern SDL_MouseButtonFlags mouseState;
extern SDL_FPoint mousePos;
extern ButtonMap mouseButtons[3];

extern TextureRef* textBufferTex;
extern Font defaultFont;

extern TextureRef* homerTex;

StudioPanel testGamePanel = {PANEL_GAME};
StudioPanel testToolbarPanel = {PANEL_TOOLBAR};
StudioPanel testExplorerPanel = {PANEL_EXPLORER};
StudioPanel testConsolePanel = {PANEL_CONSOLE};
StudioPanel testCodePanel = {PANEL_CODEEDITOR};

StudioSplit testPanelC = {false, false, &testGamePanel, &testConsolePanel, 0.8, true};
StudioSplit testPanelD = {false, true, &testCodePanel, &testPanelC, 0.2, false};
StudioSplit testPanelB = {true, false, &testPanelD, &testExplorerPanel, 0.85, false};
StudioSplit panelHead = {false, true, &testToolbarPanel, &testPanelB, 0.1, true};

void updatePanel(StudioPanel* item, SDL_FRect* area);

void updateSplit(StudioSplit* item, SDL_FRect* area){
	SDL_FRect areaA; SDL_FRect areaB;

	if(item->vert){ 	//vertical panels :
		areaA = (SDL_FRect){
			area->x, area->y, 
			area->w, area->h * item->split
		};
		areaB = (SDL_FRect){
			area->x, -1 + area->h * (1-item->split), 
			area->w, area->h * (1-item->split)
		};
	}else{		//horizontal panels ..
		areaA = (SDL_FRect){
			area->x, area->y, 
			area->w * item->split, area->h
		};
		areaB = (SDL_FRect){
			area->x + area->w * item->split, area->y, 
			area->w * (1-item->split), area->h
		};
	}

	if(item->childA){
		if(item->splitA) updateSplit((StudioSplit*)item->childA, &areaA);
		else updatePanel((StudioPanel*)item->childA, &areaA);
	}

	if(item->childB){
		if(item->splitB) updateSplit((StudioSplit*)item->childB, &areaB);
		else updatePanel((StudioPanel*)item->childB, &areaB);
	}
}

void drawPanel(StudioPanel* item, SDL_FRect* area);
void drawSplit(StudioSplit* item, SDL_FRect* area){
	SDL_FRect areaA; SDL_FRect areaB;

	if(item->vert){ 	//vertical panels :
		areaA = (SDL_FRect){
			area->x, area->y, 
			area->w, area->h * item->split
		};
		areaB = (SDL_FRect){
			area->x, -1 + area->h * (1-item->split), 
			area->w, area->h * (1-item->split)
		};
	}else{		//horizontal panels ..
		areaA = (SDL_FRect){
			area->x, area->y, 
			area->w * item->split, area->h
		};
		areaB = (SDL_FRect){
			area->x + area->w * item->split, area->y, 
			area->w * (1-item->split), area->h
		};
	}

	//float* aMatrix = genMatrix((Vector3){areaA.x, areaA.y, 0}, (Vector3){areaA.w, 1, areaA.h}, (Vector3){HALFPI, 0, 0});
	//float* bMatrix = genMatrix((Vector3){areaB.x, areaB.y, 0}, (Vector3){areaB.w, 1, areaB.h}, (Vector3){HALFPI, 0, 0});

	if(item->childA){
		if(item->splitA) drawSplit((StudioSplit*)item->childA, &areaA);
		else drawPanel((StudioPanel*)item->childA, &areaA);
	}
	//drawMeshOpenGL(planePrim, aMatrix, (SDL_FColor){1, 0, 0, 0.25}, homerTex);

	if(item->childB){
		if(item->splitB) drawSplit((StudioSplit*)item->childB, &areaB);
		else drawPanel((StudioPanel*)item->childB, &areaB);
	}
	//drawMeshOpenGL(planePrim, bMatrix, (SDL_FColor){0, 0, 1, 0.25}, homerTex);

	//free(aMatrix); free(bMatrix);
}

extern Mesh* frameBuffMesh;
extern FrameBuffer* gameBuffer;
void drawGamePanel(StudioPanel* item, SDL_FRect* area){
	(void)item;
	
	float* panelMatrix = genMatrix((Vector3){area->x + area->w * 0.5, area->y - area->h * 0.5, 0}, (Vector3){area->w * 0.5, area->h * 0.5, 1}, (Vector3){0, 0, 0});
	drawMeshOpenGL(frameBuffMesh, panelMatrix, (SDL_FColor){1, 1, 1, 1}, gameBuffer->texture);
	free(panelMatrix);
}

extern TextureRef* toolButtonTex;
void drawToolbarPanel(StudioPanel* item, SDL_FRect* area){
	(void)item;
	
	float* panelMatrix = genMatrix((Vector3){area->x, area->y, 0}, (Vector3){area->w, 1, area->h}, (Vector3){HALFPI, 0, 0});
	drawMeshOpenGL(planePrim, panelMatrix, (SDL_FColor){0.5, 0.5, 0.55, 1}, NULL);
	free(panelMatrix);

	float* placeholdMatrix = genMatrix((Vector3){area->x + area->h * 0.25, area->y - area->h * 0.25, 0}, (Vector3){area->h * 4, 1, area->h * 0.5}, (Vector3){HALFPI, 0, 0});
	drawMeshOpenGL(planePrim, placeholdMatrix, (SDL_FColor){1, 1, 1, 1}, toolButtonTex);
	free(placeholdMatrix);
}

extern DataObj *focusObject;
void drawExplorerItem(SDL_FRect* area, DataObj* item, int nodeDepth, int *idCount){
	int i = (*idCount)++;

	if(item == focusObject){
		float* backMatrix = genMatrix((Vector3){area->x, area->y - i * 0.025, 0}, (Vector3){area->h * 2, 1, 0.025}, (Vector3){HALFPI, 0, 0});
		drawMeshOpenGL(planePrim, backMatrix, (SDL_FColor){0.25, 0.75, 0.09, 1}, NULL);
		free(backMatrix);
	}
	drawGlText(&defaultFont, (Vector3){area->x + nodeDepth * 0.05, area->y - i * 0.025, 0}, item->name, 0.025, (SDL_FColor){1, 1, 1, 1});

	//if(!item->studioOpen) return;
	
	DataObj* child = item->child;
	while (child) {
		DataObj *next = child->next;
		drawExplorerItem(area, child, nodeDepth + 1, idCount);
		child = next;
	}
}

extern ClientData client;
void selectExplorerItem(Vector3 area, SDL_FPoint mouse, DataObj* item, int *idCount){
	int i = (*idCount)++;
	if(mouseButtons[0].pressed && between(mouse.y - (i * 0.05 * windowScale.y), 0, 0.05 * windowScale.y))
		focusObject = item;

	DataObj* child = item->child;
	while (child) {
		DataObj *next = child->next;
		selectExplorerItem(area, mouse, child, idCount);
		child = next;
	}
}

void updateExplorerPanel(StudioPanel* item, SDL_FRect* area){
	(void)item;
	
	//Vector3 areaScreen = glToScreen((Vector3){area->x, area->y, 0});
	//SDL_FPoint newMousePos = {areaScreen.x - mousePos.x, -(areaScreen.y - mousePos.y)};

	/*if(!between(newMousePos.x - areaScreen.x, 0, area->w * windowScale.x) && !between(newMousePos.y - areaScreen.y, 0, area->h * windowScale.y)) return;
	//printf("New mouse: x %f y %f\n", newMousePos.x, newMousePos.y);

	int idCounter = 0;
	selectExplorerItem(areaScreen, newMousePos, client.gameWorld->headObj, &idCounter);
	*/
}

void drawExplorerPanel(StudioPanel* item, SDL_FRect* area){
	(void)item;

	float* panelMatrix = genMatrix((Vector3){area->x, area->y, 0}, (Vector3){area->w, 1, area->h}, (Vector3){HALFPI, 0, 0});
	drawMeshOpenGL(planePrim, panelMatrix, (SDL_FColor){0.5, 0.5, 0.55, 1}, NULL);
	free(panelMatrix);

	int idCounter = 0;
	drawExplorerItem(area, client.gameWorld->headObj, 0, &idCounter);
}

SDL_FColor logColours[CONSOLELOG_MAX] = {
	(SDL_FColor){1, 1, 1, 1},
	(SDL_FColor){1, 0.96, 0, 1}, (SDL_FColor){1, 0.14, 0.04, 1},
	(SDL_FColor){0.65, 0.12, 0.98, 1}, (SDL_FColor){0.65, 0.65, 0.65, 1}
};

float logTextSize = 0.02;
extern ConsoleLog *consoleHead;
int consoleScroll = 0;
void drawConsolePanel(StudioPanel* item, SDL_FRect* area){
	(void)item;

	float* panelMatrix = genMatrix((Vector3){area->x, area->y, 0}, (Vector3){area->w, 1, area->h}, (Vector3){HALFPI, 0, 0});
	drawMeshOpenGL(planePrim, panelMatrix, (SDL_FColor){0.2, 0.2, 0.23, 1}, NULL);
	free(panelMatrix);

	ConsoleLog *currLog = consoleHead;
	int index = 0;
	while(currLog){
		//if(currLog->type == CONSOLELOG_EXTRA) goto logDrawSkip;
		int itemY = index - consoleScroll + 16;
		if(itemY < 0) goto logDrawSkip;

		drawGlText(
			&defaultFont, 
			(Vector3){area->x + 4*logTextSize*(currLog->count > 1), area->y - itemY * logTextSize, 0}, 
			currLog->text, 
			logTextSize, 
			logColours[currLog->type]
		);

		if(currLog->count > 1){
			char numString[8]; sprintf(numString, "x%d", currLog->count);
			drawGlText(
				&defaultFont, 
				(Vector3){area->x, area->y - itemY * logTextSize, 0}, 
				numString, 
				logTextSize, 
				logColours[currLog->type]
			);
		}

logDrawSkip:
		index++;
		currLog = currLog->next;
	}
}

/*
	PANEL_GAME, PANEL_EXPLORER,
	PANEL_PROPERTIES, PANEL_CONSOLE,
	PANEL_TOOLBAR, PANEL_CODEEDITOR,
}*/

#include "../blockcode/blockcode.h"
extern CodeBlock testCodeBlock;
void drawCodePanel(StudioPanel* item, SDL_FRect* area){
	(void)item;

	float* panelMatrix = genMatrix((Vector3){area->x, area->y, 0}, (Vector3){area->w, 1, area->h}, (Vector3){HALFPI, 0, 0});
	drawMeshOpenGL(planePrim, panelMatrix, (SDL_FColor){0.2, 0.2, 0.23, 1}, NULL);
	free(panelMatrix);

	drawCodeBlock(&testCodeBlock, (SDL_FPoint){area->x, area->y});
}

void updatePanel(StudioPanel* item, SDL_FRect* area){
	if(!item) return;

	switch(item->type){
		case PANEL_EXPLORER: updateExplorerPanel(item, area); break;
	}
}

void drawPanel(StudioPanel* item, SDL_FRect* area){
	if(!item) return;

	switch(item->type){
		case PANEL_GAME: drawGamePanel(item, area); break;
		case PANEL_EXPLORER: drawExplorerPanel(item, area); break;
		case PANEL_TOOLBAR: drawToolbarPanel(item, area); break;
		case PANEL_CONSOLE: drawConsolePanel(item, area); break;
		case PANEL_CODEEDITOR: drawCodePanel(item, area); break;
	}
}