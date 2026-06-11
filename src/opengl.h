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

typedef struct FrameBuffer{
	Uint32 frameBuff, renderBuff;
	TextureRef* texture;
} FrameBuffer;

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
void drawMeshOpenGL(Mesh* mesh, mat4 transform, SDL_FColor colour, TextureRef* texture);

//void openGlBindMesh(Mesh* mesh);

void bufferGLText(TextureRef* target, Font* font, char* text, SDL_FColor colour);

FrameBuffer* newFrameBuffer(Uint16 width, Uint16 height);
void bindFrameBuffer(FrameBuffer* item);
void freeFrameBuffer(FrameBuffer* item);

#endif