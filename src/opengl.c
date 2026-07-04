//#define GLEW_STATIC

#include <SDL3/SDL.h>
#include <SDL3_image/SDL_image.h>
#include <GL/glew.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#include <structs.h>
#include "opengl.h"
#include "loader.h"
#include "renderer.h"
#include "math.h"

#include "softwarerender/main.h"

extern SDL_Window *window;
extern SDL_Point windowScale;

extern ClientData client;
extern Camera currentCamera;
extern float* defaultMatrix;

extern Mesh* playerMesh;
extern Mesh* cubePrim;
extern SDL_FColor skyboxColour;

extern Uint32 mainShader;

const char *vertexShaderSource = NULL;
const char *fragmentShaderSource = NULL;

SDL_Window *glWindow = NULL;
SDL_Point glWindowScale = {640, 480};

extern Sint32 glLocs[GLVAL_MAX];
Uint32 glDepthTest = GL_DEPTH_TEST;

extern Uint32 VAO, VBO, EBO;

extern void HandleKeyInput();

Uint32 loadShader(char* vertPath, char* fragPath){
	char* vertSource = loadTextFile(vertPath);
	Uint32 vertShader = glCreateShader(GL_VERTEX_SHADER);
	glShaderSource(vertShader, 1, (const char**)&vertSource, NULL);
	
	char* fragSource = loadTextFile(fragPath);
	Uint32 fragShader = glCreateShader(GL_FRAGMENT_SHADER);
	glShaderSource(fragShader, 1, (const char**)&fragSource, NULL);
	
	glCompileShader(vertShader); glCompileShader(fragShader);

	free(vertSource); 
    free(fragSource);
	
	Uint32 shaderProg = glCreateProgram();
	glAttachShader(shaderProg, vertShader); glAttachShader(shaderProg, fragShader);
	glLinkProgram(shaderProg);

	glDeleteShader(vertShader); glDeleteShader(fragShader);

	glValidateProgram(shaderProg);
	int validated; glGetProgramiv(shaderProg,  GL_VALIDATE_STATUS, &validated);
	if(!validated)
		printf("Shader %d failed to compile...\n", shaderProg);
	
	return shaderProg;
}

void setGlValue(Uint32 item, bool value){
	if(value)
		glEnable(item);
	else
		glDisable(item);
}

void setGlShader(Uint32 shader){
	glUseProgram(shader);
}

bool setGlTexture(Texture* tex){
	if(!tex) return 0;
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, tex->width, tex->height, 0, GL_RGBA, GL_UNSIGNED_BYTE, tex->pixels);
	return 1;
}

extern Uint32 glBlankTex;
void drawMeshOpenGL(Mesh* mesh, mat4 transform, SDL_FColor colour, TextureRef* overrideTexture) {
	glUniformMatrix4fv(glLocs[GLVAL_WORLDMATRIX], 1, GL_FALSE, transform);

	float colourFloat[4] = {colour.r, colour.g, colour.b, colour.a};
	glUniform4fv(glLocs[GLVAL_MULTCOLOUR], 1, colourFloat);

	glBufferData(GL_ARRAY_BUFFER, sizeof(MeshVert) * mesh->vertCount, mesh->verts, GL_STATIC_DRAW);
    
	if (overrideTexture && overrideTexture->glLoc) {
		glBindTexture(GL_TEXTURE_2D, overrideTexture->glLoc);
	} else {
		glBindTexture(GL_TEXTURE_2D, glBlankTex);
	}

	TextureRef* lastTex = NULL;
	GLuint currentTex = glBlankTex;

	GLuint *indices = malloc(sizeof(GLuint) * mesh->faceCount * 3);
	Uint32 idxCount = 0;

	for (Uint32 i = 0; i < mesh->faceCount; ++i) {
		MeshFace *face = &mesh->faces[i];
		MeshMaterial noMat = {0};
		MeshMaterial *currMaterial = face->material;

		if (!currMaterial) {
			currMaterial = &noMat;
		}

		bool newTex = currMaterial->texture != lastTex;

		if(newTex && idxCount > 0){
			glBindTexture(GL_TEXTURE_2D, overrideTexture ? overrideTexture->glLoc : currentTex);
			glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(GLuint) * idxCount, indices, GL_STATIC_DRAW);
			glDrawElements(GL_TRIANGLES, idxCount, GL_UNSIGNED_INT, 0);
			idxCount = 0;
		}

		if(newTex){
			currentTex = currMaterial->texture && currMaterial->texture->glLoc ? currMaterial->texture->glLoc : glBlankTex;
			lastTex = currMaterial->texture;
		}

		indices[idxCount++] = face->vertA;
		indices[idxCount++] = face->vertB;
        indices[idxCount++] = face->vertC;
    }

    if (idxCount > 0) {
        glBindTexture(GL_TEXTURE_2D, overrideTexture ? overrideTexture->glLoc : currentTex);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(GLuint) * idxCount, indices, GL_STATIC_DRAW);
        glDrawElements(GL_TRIANGLES, idxCount, GL_UNSIGNED_INT, 0);
    }

    free(indices);

	glBindTexture(GL_TEXTURE_2D, 0);
}

void deleteBuffer(Uint32 id){
	glDeleteBuffers(1, &id);
}

void genVBO(Mesh* mesh, Uint32* id){ 
	/*glGenBuffers(1, id);
	glBindBuffer(GL_ARRAY_BUFFER, *id);
	glBufferData(GL_ARRAY_BUFFER, mesh->vertCount * sizeof(MeshVert), mesh->verts, GL_STATIC_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, 0);*/
}
void bindVBO(Uint32 id){
	glBindBuffer(GL_ARRAY_BUFFER, id);
}
void unbindVBO(){
	glBindBuffer(GL_ARRAY_BUFFER, 0);
}

void genEBO(Mesh* mesh, Uint32* id){
	// TODO? :
	// this will not be valid because mesh->faces has a material alongside everything else...
	// so its not even needed since we already do the materials and batching in drawMeshOpenGL
	glGenBuffers(1, id);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, *id);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, mesh->faceCount * sizeof(MeshFace), mesh->faces, GL_STATIC_DRAW);
}
void bindEBO(Uint32 id){
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, id);
}
void unbindEBO(){
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
}

void openGlGenBuffers(Mesh* mesh){
    if(!mesh) return;
    
	genVBO(mesh, &mesh->vertBuffer);
}

float bufferGLText(TextureRef* target, Font* font, char* text, float size){
	Texture* tex = target->texture;
	if(tex)
		freeRasterTexture(tex);
	SDL_Point scale = {font->renderSize.x + max(font->kerning.x * size * strlen(text), font->renderSize.x * size), font->renderSize.y * size};
	Texture* newTex = newRasterTexture(scale.x, scale.y);
	target->texture = newTex;

	clearTex(newTex, 0x00FFFFFF);
	drawRasterText(newTex, font, text, 0, 0, size, 0xFFFFFFFF);

	glBindTexture(GL_TEXTURE_2D, target->glLoc);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, newTex->width, newTex->height, 0, GL_RGBA, GL_UNSIGNED_BYTE, newTex->pixels);
	glBindTexture(GL_TEXTURE_2D, 0);

	return (float)scale.y / scale.x;
}

extern TextureRef* textBufferTex;
extern Mesh* planePrim;
void drawGlText(Font* font, Vector3 pos, char* text, float size, SDL_FColor colour){
	float textRatio = bufferGLText(textBufferTex, font, text, 2);
	float* textMatrix = genMatrix((Vector3){pos.x, pos.y, pos.z}, (Vector3){size / textRatio, 1, size}, (Vector3){HALFPI, 0, 0});
	drawMeshOpenGL(planePrim, textMatrix, colour, textBufferTex);
	free(textMatrix);
}

FrameBuffer* newFrameBuffer(Uint16 width, Uint16 height){
	FrameBuffer* newFB = calloc(1, sizeof(FrameBuffer));
	if(!newFB) return NULL;

	TextureRef* fbTexture = calloc(1, sizeof(TextureRef)); 
	newFB->texture = fbTexture;
	newFB->width = width; newFB->height = height;

	glGenFramebuffers(1, &newFB->frameBuff);
	glBindFramebuffer(GL_FRAMEBUFFER, newFB->frameBuff);

	glGenTextures(1, &fbTexture->glLoc);
	glBindTexture(GL_TEXTURE_2D, fbTexture->glLoc);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST); glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE); glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, fbTexture->glLoc, 0);

	glGenRenderbuffers(1, &newFB->renderBuff);
	glBindRenderbuffer(GL_RENDERBUFFER, newFB->renderBuff);
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, width, height);
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, newFB->renderBuff);
	glBindTexture(GL_TEXTURE_2D, 0);

	int fbStatus = glCheckFramebufferStatus(GL_FRAMEBUFFER);
	if(fbStatus != GL_FRAMEBUFFER_COMPLETE)
		printf("Framebuffer error: %d\n", fbStatus);

	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glBindRenderbuffer(GL_RENDERBUFFER, 0);

	return newFB;
}

void bindFrameBuffer(FrameBuffer* item){
	glBindFramebuffer(GL_FRAMEBUFFER, item ? item->frameBuff : 0);
	if(item) glViewport(0, 0, item->width, item->height);
	else glViewport(0, 0, windowScale.x, windowScale.y);
}

void freeFrameBuffer(FrameBuffer* item){
	if(!item) return;
	if(item->texture) freeTexture(item->texture); 
	
    glDeleteFramebuffers(1, &item->frameBuff); 
    glDeleteRenderbuffers(1, &item->renderBuff);
	free(item);
}