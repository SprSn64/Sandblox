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

extern Uint32 mainShader; extern Uint32 flatShader;

const char *vertexShaderSource = NULL;
const char *fragmentShaderSource = NULL;

SDL_Window *glWindow = NULL;
SDL_Point glWindowScale = {640, 480};

extern Sint32 glLocs[GLVAL_MAX];
Uint32 glDepthTest = GL_DEPTH_TEST;

extern Uint32 VAO, VBO, EBO;

extern void HandleKeyInput();

Uint32 loadShader(char* vertPath, char* fragPath){
	const char* vertSource = loadTextFile(vertPath);
	Uint32 vertShader = glCreateShader(GL_VERTEX_SHADER);
	glShaderSource(vertShader, 1, &vertSource, NULL);
	
	const char* fragSource = loadTextFile(fragPath);
	Uint32 fragShader = glCreateShader(GL_FRAGMENT_SHADER);
	glShaderSource(fragShader, 1, &fragSource, NULL);
	
	glCompileShader(vertShader); glCompileShader(fragShader);
	
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

    float colourFloat[4] = { colour.r, colour.g, colour.b, colour.a };
    glUniform4fv(glLocs[GLVAL_MULTCOLOUR], 1, colourFloat);

    glBufferData(GL_ARRAY_BUFFER, sizeof(MeshVert) * mesh->vertCount, mesh->verts, GL_STATIC_DRAW);
    
    if (overrideTexture && overrideTexture->glLoc)
        glBindTexture(GL_TEXTURE_2D, overrideTexture->glLoc);
    else
        glBindTexture(GL_TEXTURE_2D, glBlankTex);

    char lastTex[256] = "";
    GLuint currentTex = glBlankTex;

    GLuint *indices = malloc(sizeof(GLuint) * mesh->faceCount * 3);
    Uint32 idxCount = 0;

    for (Uint32 i = 0; i < mesh->faceCount; ++i) {
        MeshFace *face = &mesh->faces[i];
        char *texPath = face->material.tex;

        bool newTexture = strcmp(lastTex, texPath) != 0;

        if (newTexture && idxCount > 0) {
            glBindTexture(GL_TEXTURE_2D, overrideTexture ? overrideTexture->glLoc : currentTex );
            glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(GLuint) * idxCount, indices, GL_STATIC_DRAW);
            glDrawElements(GL_TRIANGLES, idxCount, GL_UNSIGNED_INT, 0);
            idxCount = 0;
        }

        if (newTexture) {
            TextureRef *tref = loadTexture(texPath, false);
            currentTex = tref && tref->glLoc ? tref->glLoc : glBlankTex;
            strcpy(lastTex, texPath);
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

void genVBO(Mesh* mesh, Uint32 id){ //crashes
	glGenBuffers(1, &id);
	glBindBuffer(GL_ARRAY_BUFFER, id);
	glBufferData(GL_ARRAY_BUFFER, mesh->vertCount * sizeof(MeshVert), mesh->verts, GL_STATIC_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
}
void bindVBO(Uint32 id){
	glBindBuffer(GL_ARRAY_BUFFER, id);
}
void unbindVBO(){
	glBindBuffer(GL_ARRAY_BUFFER, 0);
}

void genEBO(Mesh* mesh, Uint32 id){
	glGenBuffers(1, &id);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, id);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, mesh->faceCount * sizeof(MeshFace), mesh->faces, GL_STATIC_DRAW);
}
void bindEBO(Uint32 id){
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, id);
}
void unbindEBO(){
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
}

void openGlGenBuffers(Mesh* mesh){
	(void)mesh;
	//how the fuck do i get it working

	//mesh->vertArray = 0;
	//genVBO(mesh, mesh->vertBuffer);
	//mesh->eleBuffer = genEBO(mesh, 0);
}