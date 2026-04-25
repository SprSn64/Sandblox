#ifndef GAMEOPENGL_H
#define GAMEOPENGL_H

#include <structs.h>
#include "renderer.h"

typedef enum glValueLocations{
	GLVAL_WORLDMATRIX, GLVAL_VIEWMATRIX, GLVAL_PROJMATRIX,
	GLVAL_LIGHTNORM, GLVAL_LIGHTCOLOUR, GLVAL_AMBCOLOUR, GLVAL_MULTCOLOUR,
	GLVAL_CAMERANORM, GLVAL_RESOLUTION, GLVAL_TEXTURE0,
	GLVAL_MAX
} glValueLocations;

bool initOpenGL();
void updateOpenGL();
void endUpdateOpenGL();
void cleanupOpenGL();

Uint32 loadShader(char* vertPath, char* fragPath);
bool setGlTexture(Texture* tex);

void setGlValue(Uint32 item, bool value);
void setGlShader(Uint32 shader);

void openGlGenBuffers(Mesh* mesh);

//swap SDL_Texture with Texture from softwarerenderer soon
void drawMeshOpenGL(Mesh* mesh, mat4 transform, SDL_FColor colour, SDL_Texture* texture);

//void openGlBindMesh(Mesh* mesh);

#endif