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

Uint32 mainShader; Uint32 flatShader;

const char *vertexShaderSource = NULL;
const char *fragmentShaderSource = NULL;

Texture* glTestTex;
Uint32 glTestTexID;

SDL_Window *glWindow = NULL;
SDL_Point glWindowScale = {640, 480};

Sint32 worldLoc, viewLoc,  projLoc;
Sint32 glLocs[GLVAL_MAX];
Uint32 glDepthTest = GL_DEPTH_TEST;

Uint32 VAO, VBO, EBO;

extern void HandleKeyInput();

MeshVert testGlVerts[5] = {
	(MeshVert){(Vector3){-1, -1, 1}, (Vector3){-1, -1, 1}, (SDL_FPoint){0, 0}, (SDL_FColor){1, 0, 0, 1}},
	(MeshVert){(Vector3){-1, -1, -1}, (Vector3){-1, -1, -1}, (SDL_FPoint){0, 0}, (SDL_FColor){0, 1, 0, 1}},
	(MeshVert){(Vector3){1, -1, -1}, (Vector3){1, -1, -1}, (SDL_FPoint){0, 0}, (SDL_FColor){1, 1, 0, 1}},
	(MeshVert){(Vector3){1, -1, 1}, (Vector3){1, -1, 1}, (SDL_FPoint){0, 0}, (SDL_FColor){0, 0, 1, 1}},
	(MeshVert){(Vector3){0, 1, 0}, (Vector3){0, 1, 0}, (SDL_FPoint){0, 0}, (SDL_FColor){1, 0, 1, 1}}
};
MeshFace testGlFaces[6] = {
	(MeshFace){0, 1, 4}, (MeshFace){1, 2, 4}, (MeshFace){2, 3, 4}, (MeshFace){3, 0, 4},
	(MeshFace){1, 0, 2}, (MeshFace){3, 2, 0}
};
Uint32 tempVertCount = 5;
Uint32 tempTriCount = 6;

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
	
	return shaderProg;
}

float *projMatrixOpenGL(float fov, float aspect, float zNear, float zFar){
	float *output = calloc(1, sizeof(mat4));
	float fovTan = SDL_tan(fov / 2 * DEG2RAD);

	output[0] = 1/(fovTan*aspect);
	output[5] = 1/fovTan;
	output[10] = -(zFar + zNear) / (zFar - zNear);
	output[14] = -1;
	output[11] = -zNear;

	//output[10] = -(zFar + zNear) / range;
	//output[14] = -(2 * zFar * zNear) / range;

	return output;
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

bool initOpenGL(){
	glWindow = SDL_CreateWindow("Sandblox (3D OpenGL)", glWindowScale.x, glWindowScale.y, SDL_WINDOW_RESIZABLE | SDL_WINDOW_OPENGL);
	SDL_GL_CreateContext(glWindow);
	if(!SDL_GL_CreateContext(glWindow)){
		printf("OpenGL initiation failed!\n");
		return 0;
	}
	glewInit();
	
	SDL_SetWindowParent(glWindow, window); //SDL_SetWindowModal(glWindow, true);
	SDL_SetWindowMinimumSize(glWindow, 320, 240);
	
	mainShader = loadShader("assets/shaders/default.vert", "assets/shaders/default.frag");
	flatShader = loadShader("assets/shaders/default.vert", "assets/shaders/unshaded.frag");

	printf("Main Shader: %d, Flat Shader: %d\n", mainShader, flatShader);
	
	glGenVertexArrays(1, &VAO); glBindVertexArray(VAO);	
	glGenBuffers(1, &VBO); glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glGenBuffers(1, &EBO); glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
	
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(MeshFace) * tempTriCount, testGlFaces, GL_STATIC_DRAW); 
	glBufferData(GL_ARRAY_BUFFER, sizeof(MeshVert) * tempVertCount, testGlVerts, GL_STATIC_DRAW);

	//glBindBuffer(GL_ARRAY_BUFFER, 0); glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
	
	//glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(MeshFace) * playerMesh->faceCount, playerMesh->faces, GL_STATIC_DRAW); 
	//glBufferData(GL_ARRAY_BUFFER, sizeof(MeshVert) * playerMesh->vertCount, playerMesh->verts, GL_STATIC_DRAW);
	
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
	glUniform1i(glLocs[GLVAL_TEXTURE0], 0);
	
	SDL_GL_SetSwapInterval(1);
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_CULL_FACE); glCullFace(GL_BACK); glFrontFace(GL_CCW);
	glClearColor(skyboxColour.r, skyboxColour.g, skyboxColour.b, 1);

	glUniformMatrix4fv(glLocs[GLVAL_WORLDMATRIX], 1, GL_FALSE, defaultMatrix);

	glTestTex = loadRasterTexture("assets/textures/cows.png");
	glGenTextures(1, &glTestTexID);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, glTestTexID);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	setGlTexture(glTestTex);
	glGenerateMipmap(GL_TEXTURE_2D);

	glBindTexture(GL_TEXTURE_2D, 0);
	
	return 1;
}

extern void studioCameraUpdate(Camera* cam);
extern Vector3 lightNormal;
extern SDL_FColor lightColour;
extern SDL_FColor lightAmbient;

void updateOpenGL(){
	SDL_GetWindowSize(glWindow, &glWindowScale.x, &glWindowScale.y);
	glViewport(0, 0, glWindowScale.x, glWindowScale.y);

	HandleKeyInput();
	if(client.pause)
		studioCameraUpdate(client.gameWorld->currCamera);

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glUseProgram(mainShader);

	float resFloat[2] = {glWindowScale.x, glWindowScale.y};
	glUniform2fv(glLocs[GLVAL_RESOLUTION], 1, resFloat);

	glUniform3fv(glLocs[GLVAL_LIGHTNORM], 1, (float*)&lightNormal);
	Vector3 cameraNormal = rotToNorm3(client.gameWorld->currCamera->rot);
	glUniform3fv(glLocs[GLVAL_CAMERANORM], 1, (float*)&cameraNormal);

	glUniform4fv(glLocs[GLVAL_LIGHTCOLOUR], 1, (float*)&lightColour);
	glUniform4fv(glLocs[GLVAL_AMBCOLOUR], 1, (float*)&lightAmbient);

	glBindTexture(GL_TEXTURE_2D, glTestTexID);
	
	glUniformMatrix4fv(glLocs[GLVAL_PROJMATRIX], 1, GL_FALSE, currentCamera.proj);
	glUniformMatrix4fv(glLocs[GLVAL_VIEWMATRIX], 1, GL_FALSE, currentCamera.transform);
	//glDrawElements(GL_TRIANGLES, tempTriCount * 3, GL_UNSIGNED_INT, 0); 
}

void endUpdateOpenGL(){
	SDL_GL_SwapWindow(glWindow);
}

void cleanupOpenGL(){
	glDeleteProgram(mainShader);
	glDeleteBuffers(1, &VAO); glDeleteBuffers(1, &VBO); glDeleteBuffers(1, &EBO);
	glDeleteTextures(1, &glTestTexID);
}

void drawMeshOpenGL(Mesh* mesh, mat4 transform, SDL_FColor colour, SDL_Texture* texture){
	(void)texture;
	glUniformMatrix4fv(glLocs[GLVAL_WORLDMATRIX], 1, GL_FALSE, transform);

	float colourFloat[4] = {colour.r, colour.g, colour.b, colour.a};
	glUniform4fv(glLocs[GLVAL_MULTCOLOUR], 1, colourFloat);

	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(MeshFace) * mesh->faceCount, mesh->faces, GL_STATIC_DRAW); 
	glBufferData(GL_ARRAY_BUFFER, sizeof(MeshVert) * mesh->vertCount, mesh->verts, GL_STATIC_DRAW);

	//glBindBuffer(GL_ARRAY_BUFFER, mesh->vertArray);
	//glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mesh->eleBuffer);

	tempTriCount = mesh->faceCount;
	tempVertCount = mesh->vertCount;
	
	glDrawElements(GL_TRIANGLES, mesh->faceCount * 3, GL_UNSIGNED_INT, 0);

	//glBindBuffer(GL_ARRAY_BUFFER, 0);
	//glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
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