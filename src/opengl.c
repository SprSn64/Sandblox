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

extern SDL_Window *window;
extern SDL_Point windowScale;

extern ClientData client;
extern Camera currentCamera;
extern float* defaultMatrix;

extern Mesh* playerMesh;
extern Mesh* cubePrim;
extern SDL_FColor skyboxColour;

Uint32 mainShader;

const char *vertexShaderSource = NULL;
const char *fragmentShaderSource = NULL;

SDL_Window *glWindow = NULL;
SDL_Point glWindowScale = {640, 480};

Sint32 worldLoc, viewLoc,  projLoc;
Sint32 glLocs[GLVAL_MAX];

float* worldMat = NULL;
float* viewMat;

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
	
	float range = zNear - zFar;
	float fovTan = SDL_tan(fov / 2 * DEG2RAD);
	
	output[0] = -1 / (fovTan * aspect);
	output[5] = 1 / fovTan;
	output[10] = (-zNear - zFar) / range;
	output[11] = 2 * zNear * zFar / range;
	output[14] = 1;

	return output;
}

bool initOpenGL(){
	glWindow = SDL_CreateWindow("Sandblox (3D OpenGL)", glWindowScale.x, glWindowScale.y, SDL_WINDOW_RESIZABLE | SDL_WINDOW_OPENGL);
	SDL_GL_CreateContext(glWindow);
	if(!SDL_GL_CreateContext(glWindow)){
		printf("OpenGL initiation failed!\n");
		return 0;
	}
	if(glewInit() != GLEW_OK){
		printf("Glew initiation failed!\n");
		return 0;
	}
	printf("Glew initiation successful!\n");
	
	SDL_SetWindowParent(glWindow, window); //SDL_SetWindowModal(glWindow, true);
	SDL_SetWindowMinimumSize(glWindow, 320, 240);
	
	mainShader = loadShader("assets/shaders/default.vert", "assets/shaders/default.frag");
	
	glGenVertexArrays(1, &VAO); glBindVertexArray(VAO);	
	glGenBuffers(1, &VBO); glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glGenBuffers(1, &EBO); glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
	
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(MeshFace) * tempTriCount, testGlFaces, GL_STATIC_DRAW); 
	glBufferData(GL_ARRAY_BUFFER, sizeof(MeshVert) * tempVertCount, testGlVerts, GL_STATIC_DRAW);
	
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
	
	SDL_GL_SetSwapInterval(1);
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_CULL_FACE); glCullFace(GL_BACK); glFrontFace(GL_CCW);
	glClearColor(skyboxColour.r, skyboxColour.g, skyboxColour.b, 1);

	glUniformMatrix4fv(glLocs[GLVAL_WORLDMATRIX], 1, GL_FALSE, defaultMatrix);
	
	return 1;
}

void updateOpenGL(){
	SDL_GetWindowSize(glWindow, &glWindowScale.x, &glWindowScale.y);
	glViewport(0, 0, glWindowScale.x, glWindowScale.y);

	HandleKeyInput();

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glUseProgram(mainShader);

	float lightNormFloat[3] = {0.25, 0.42, 0.33};
	glUniform3fv(glLocs[GLVAL_LIGHTNORM], 1, lightNormFloat);
	Vector3 cameraNormal = rotToNorm3(client.gameWorld->currCamera->rot);
	float cameraNormFloat[3] = {cameraNormal.x, cameraNormal.y, cameraNormal.z};
	glUniform3fv(glLocs[GLVAL_CAMERANORM], 1, cameraNormFloat);

	float lightColourFloat[4] = {1, 1, 1, 1};
	glUniform4fv(glLocs[GLVAL_LIGHTCOLOUR], 1, lightColourFloat);
	float ambColourFloat[4] = {0.25, 0.25, 0.3, 1};
	glUniform4fv(glLocs[GLVAL_AMBCOLOUR], 1, ambColourFloat);
	
	//projMat = projMatrix(90, (float)glWindowScale.x/glWindowScale.y, 0.1, 100); //world flipped?
	//float* worldMat = genMatrix(client.gameWorld->currPlayer->pos, client.gameWorld->currPlayer->scale, client.gameWorld->currPlayer->rot);
	
	float* viewMatRotate = translateMatrix(defaultMatrix, vec3Mult(currentCamera.pos, (Vector3){-1, -1, 1}));
	viewMat = rotateMatrix(viewMatRotate, vec3Mult(currentCamera.rot, (Vector3){1, 1, 1}), ROT_YXZ);
	free(viewMatRotate);
	
	glUniformMatrix4fv(glLocs[GLVAL_PROJMATRIX], 1, GL_FALSE, currentCamera.proj);
	glUniformMatrix4fv(glLocs[GLVAL_VIEWMATRIX], 1, GL_FALSE, viewMat);//currentCamera.transform);
	glDrawElements(GL_TRIANGLES, tempTriCount * 3, GL_UNSIGNED_INT, 0); 
	
	//free(worldMat);
	free(viewMat);
	
	SDL_GL_SwapWindow(glWindow);
}

void cleanupOpenGL(){
	glDeleteProgram(mainShader);
	glDeleteBuffers(1, &VAO); glDeleteBuffers(1, &VBO); glDeleteBuffers(1, &EBO);
}

void drawMeshOpenGL(Mesh* mesh, mat4 transform, SDL_FColor colour, SDL_Texture* texture){
	(void)texture;
	glUniformMatrix4fv(glLocs[GLVAL_WORLDMATRIX], 1, GL_FALSE, transform);

	float colourFloat[4] = {colour.r, colour.g, colour.b, colour.a};
	glUniform4fv(glLocs[GLVAL_MULTCOLOUR], 1, colourFloat);

	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(MeshFace) * mesh->faceCount, mesh->faces, GL_STATIC_DRAW); 
	glBufferData(GL_ARRAY_BUFFER, sizeof(MeshVert) * mesh->vertCount, mesh->verts, GL_STATIC_DRAW);

	//glBindVertexArray(mesh->vertArray);

	tempTriCount = mesh->faceCount;
	tempVertCount = mesh->vertCount;
	
	glDrawElements(GL_TRIANGLES, mesh->faceCount * 3, GL_UNSIGNED_INT, 0);

	//glBindVertexArray(0);
}

float* vertsToArray(Mesh* mesh){
	float* vertArray = malloc(mesh->vertCount * 12 * sizeof(float));

	for(Uint32 i=0; i<mesh->vertCount; i++){
		Uint32 index = i * 12;
		MeshVert* currVert = &mesh->verts[index];

		vertArray[index] = currVert->pos.x; vertArray[index+1] = currVert->pos.y; vertArray[index+2] = currVert->pos.z;
		vertArray[index+3] = currVert->norm.x; vertArray[index+4] = currVert->norm.y; vertArray[index+5] = currVert->norm.z;
		vertArray[index+6] = currVert->uv.x; vertArray[index+7] = currVert->uv.y;
		vertArray[index+8] = currVert->colour.r; vertArray[index+9] = currVert->colour.g; vertArray[index+10] = currVert->colour.b; vertArray[index+10] = currVert->colour.a;
	}

	return vertArray;
}

void deleteBuffer(Uint32 id){
	glDeleteBuffers(1, &id);
}

void genVBO(Mesh* mesh, Uint32 id){
	float* vertArray = vertsToArray(mesh);

	glGenBuffers(1, &id);
	glBindBuffer(GL_ARRAY_BUFFER, id);
	glBufferData(GL_ARRAY_BUFFER, mesh->vertCount * sizeof(MeshVert), vertArray, GL_STATIC_DRAW);

	free(vertArray);
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
	//mesh->vertBuffer = genVBO(mesh, 0);
	//mesh->eleBuffer = genEBO(mesh, 0);
}