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

extern float aspectRatio;
extern float* defaultMatrix;
extern SDL_Point windowScale;
extern Mesh *planePrim;

extern TextureRef* homerTex;

StudioPanel testGamePanel = {PANEL_GAME};
StudioPanel testToolbarPanel = {PANEL_TOOLBAR};
StudioPanel testExplorerPanel = {PANEL_EXPLORER};

//StudioSplit testPanelC = {false, false, NULL, NULL, 0.5, false};
StudioSplit testPanelB = {false, false, &testGamePanel, &testExplorerPanel, 0.85, false};
StudioSplit panelHead = {false, true, &testToolbarPanel, &testPanelB, 0.1, true};

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

	//drawMeshOpenGL(planePrim, aMatrix, (SDL_FColor){1, 0, 0, 0.25}, homerTex);
	if(item->childA){
		if(item->splitA) drawSplit((StudioSplit*)item->childA, &areaA);
		else drawPanel((StudioPanel*)item->childA, &areaA);
	}

	//drawMeshOpenGL(planePrim, bMatrix, (SDL_FColor){0, 0, 1, 0.25}, homerTex);
	if(item->childB){
		if(item->splitB) drawSplit((StudioSplit*)item->childB, &areaB);
		else drawPanel((StudioPanel*)item->childB, &areaB);
	}

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

extern TextureRef* textBufferTex;
extern Font defaultFont;
void drawExplorerPanel(SDL_FRect* area){
	float* panelMatrix = genMatrix((Vector3){area->x, area->y, 0}, (Vector3){area->w, 1, area->h}, (Vector3){HALFPI, 0, 0});
	drawMeshOpenGL(planePrim, panelMatrix, (SDL_FColor){0.5, 0.5, 0.55, 1}, NULL);
	free(panelMatrix);

	float textScale = bufferGLText(textBufferTex, &defaultFont, "The quick brown fox\0", 2, (SDL_FColor){1, 1, 1, 1});
	float* textMatrix = genMatrix((Vector3){area->x, area->y, 0}, (Vector3){0.12 * aspectRatio * 2, 1, 0.12 * 4 * textScale}, (Vector3){HALFPI, 0, 0});
	drawMeshOpenGL(planePrim, textMatrix, (SDL_FColor){1, 1, 1, 1}, textBufferTex);
	free(textMatrix);
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
	}
}