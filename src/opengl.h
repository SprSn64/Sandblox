#ifndef GAMEOPENGL_H
#define GAMEOPENGL_H

#include <structs.h>
#include "renderer.h"

typedef enum glValueLocations{
	GLVAL_WORLDMATRIX, GLVAL_VIEWMATRIX, GLVAL_PROJMATRIX,
	GLVAL_LIGHTNORM, GLVAL_LIGHTCOLOUR, GLVAL_AMBCOLOUR, GLVAL_MULTCOLOUR,
	GLVAL_MAX
} glValueLocations;

bool initOpenGL();
void updateOpenGL();
void cleanupOpenGL();

//void drawMeshOpenGL(Mesh* mesh, mat4 transform, SDL_FColor colour, SDL_Texture* texture);

//void openGlBindMesh(Mesh* mesh);

#endif