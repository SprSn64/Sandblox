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
extern SDL_FPoint mousePos;
extern Mesh *planePrim;

extern TextureRef* textBufferTex;
extern Font defaultFont;

extern TextureRef* homerTex;

StudioPanel testGamePanel = {PANEL_GAME};
StudioPanel testToolbarPanel = {PANEL_TOOLBAR};
StudioPanel testExplorerPanel = {PANEL_EXPLORER};
StudioPanel testConsolePanel = {PANEL_CONSOLE};

StudioSplit testPanelC = {false, false, &testGamePanel, &testConsolePanel, 0.8, true};
StudioSplit testPanelB = {true, false, &testPanelC, &testExplorerPanel, 0.85, false};
StudioSplit panelHead = {false, true, &testToolbarPanel, &testPanelB, 0.1, true};

void updatePanel(StudioPanel* item, SDL_FRect* area){
	if(!withinRect(mousePos, *area)) return;
}

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
void drawGamePanel(SDL_FRect* area){
	float* panelMatrix = genMatrix((Vector3){area->x + area->w * 0.5, area->y - area->h * 0.5, 0}, (Vector3){area->w * 0.5, area->h * 0.5, 1}, (Vector3){0, 0, 0});
	drawMeshOpenGL(frameBuffMesh, panelMatrix, (SDL_FColor){1, 1, 1, 1}, gameBuffer->texture);
	free(panelMatrix);
}

extern TextureRef* toolButtonTex;
void drawToolbarPanel(SDL_FRect* area){
	float* panelMatrix = genMatrix((Vector3){area->x, area->y, 0}, (Vector3){area->w, 1, area->h}, (Vector3){HALFPI, 0, 0});
	drawMeshOpenGL(planePrim, panelMatrix, (SDL_FColor){0.5, 0.5, 0.55, 1}, NULL);
	free(panelMatrix);

	float* placeholdMatrix = genMatrix((Vector3){area->x + area->h * 0.25, area->y - area->h * 0.25, 0}, (Vector3){area->h * 2, 1, area->h * 0.5}, (Vector3){HALFPI, 0, 0});
	drawMeshOpenGL(planePrim, placeholdMatrix, (SDL_FColor){1, 1, 1, 1}, toolButtonTex);
	free(placeholdMatrix);
}

void drawExplorerItem(SDL_FRect* area, DataObj* item, int nodeDepth, int *idCount){
	int i = (*idCount)++;

	float textRatio = bufferGLText(textBufferTex, &defaultFont, item->name, 2, (SDL_FColor){1, 1, 1, 1});
	float* textMatrix = genMatrix((Vector3){area->x + nodeDepth * 0.05, area->y - i * 0.025, 0}, (Vector3){0.025 / textRatio, 1, 0.025}, (Vector3){HALFPI, 0, 0});
	drawMeshOpenGL(planePrim, textMatrix, (SDL_FColor){1, 1, 1, 1}, textBufferTex);
	free(textMatrix);

	//if(!item->studioOpen) return;
	
	DataObj* child = item->child;
	while (child) {
		DataObj *next = child->next;
		drawExplorerItem(area, child, nodeDepth + 1, idCount);
		child = next;
	}
}

extern ClientData client;
void drawExplorerPanel(SDL_FRect* area){
	float* panelMatrix = genMatrix((Vector3){area->x, area->y, 0}, (Vector3){area->w, 1, area->h}, (Vector3){HALFPI, 0, 0});
	drawMeshOpenGL(planePrim, panelMatrix, (SDL_FColor){0.5, 0.5, 0.55, 1}, NULL);
	free(panelMatrix);

	/*float textRatio = bufferGLText(textBufferTex, &defaultFont, "The quick brown fox\0", 2, (SDL_FColor){1, 1, 1, 1});
	float* textMatrix = genMatrix((Vector3){area->x, area->y, 0}, (Vector3){0.5, 1, 0.5 * textRatio}, (Vector3){HALFPI, 0, 0});
	drawMeshOpenGL(planePrim, textMatrix, (SDL_FColor){1, 1, 1, 1}, textBufferTex);
	free(textMatrix);*/

	int idCounter = 0;
	drawExplorerItem(area, client.gameWorld->headObj, 0, &idCounter);
}

SDL_FColor logColours[CONSOLELOG_MAX] = {
	(SDL_FColor){1, 1, 1, 1},
	(SDL_FColor){1, 0.96, 0, 1}, (SDL_FColor){1, 0.14, 0.04, 1},
	(SDL_FColor){0.65, 0.65, 0.65, 1}
};

float logTextSize = 0.02;
extern ConsoleLog *consoleHead;
int consoleScroll = 0;
void drawConsolePanel(SDL_FRect* area){
	float* panelMatrix = genMatrix((Vector3){area->x, area->y, 0}, (Vector3){area->w, 1, area->h}, (Vector3){HALFPI, 0, 0});
	drawMeshOpenGL(planePrim, panelMatrix, (SDL_FColor){0.2, 0.2, 0.23, 1}, NULL);
	free(panelMatrix);

	ConsoleLog *currLog = consoleHead;
	int index = 0;
	while(currLog){
		//if(currLog->type == CONSOLELOG_EXTRA) goto logDrawSkip;
		int itemY = index - consoleScroll + 16;
		if(itemY < 0) goto logDrawSkip;

		float textRatio = bufferGLText(textBufferTex, &defaultFont, currLog->text, 2, (SDL_FColor){1, 1, 1, 1});
		float* textMatrix = genMatrix((Vector3){area->x + 4*logTextSize*(currLog->count > 1), area->y - itemY * logTextSize, 0}, (Vector3){logTextSize / textRatio, 1, logTextSize}, (Vector3){HALFPI, 0, 0});
		drawMeshOpenGL(planePrim, textMatrix, logColours[currLog->type], textBufferTex);
		free(textMatrix);

		if(currLog->count > 1){
			char numString[8]; sprintf(numString, "x%d", currLog->count);
			float numRatio = bufferGLText(textBufferTex, &defaultFont, numString, 2, (SDL_FColor){1, 1, 1, 1});
			float* numMatrix = genMatrix((Vector3){area->x, area->y - itemY * logTextSize, 0}, (Vector3){logTextSize / numRatio, 1, logTextSize}, (Vector3){HALFPI, 0, 0});
			drawMeshOpenGL(planePrim, numMatrix, logColours[currLog->type], textBufferTex);
			free(numMatrix);
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

void drawPanel(StudioPanel* item, SDL_FRect* area){
	if(!item) return;

	switch(item->type){
		case PANEL_GAME: drawGamePanel(area); break;
		case PANEL_EXPLORER: drawExplorerPanel(area); break;
		case PANEL_TOOLBAR: drawToolbarPanel(area); break;
		case PANEL_CONSOLE: drawConsolePanel(area); break;
	}
}