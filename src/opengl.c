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

float* viewMat;

const char *vertexShaderSource = NULL;
const char *fragmentShaderSource = NULL;

SDL_Window *glWindow = NULL;
SDL_Point glWindowScale = {640, 480};

Sint32 worldLoc, viewLoc,  projLoc;
Sint32 glLocs[GLVAL_MAX];

float* worldMat = NULL;

Uint32 VAO, VBO, EBO;

float *rotateMatrixEvil(mat4 matrix, Vector3 angle);

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
	
	return shaderProg;
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
	
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(MeshFace) * playerMesh->faceCount, playerMesh->faces, GL_STATIC_DRAW); 
	glBufferData(GL_ARRAY_BUFFER, sizeof(MeshVert) * playerMesh->vertCount, playerMesh->verts, GL_STATIC_DRAW);
	
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
	glLocs[GLVAL_MULTCOLOUR] = glGetUniformLocation(mainShader, "multColour");
	
	//float multColour[4] = {1, 1, 1, 1};
	//glUniform4fv(glLocs[GLVAL_MULTCOLOUR], 1, multColour);
	
	SDL_GL_SetSwapInterval(1);
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_CULL_FACE); glCullFace(GL_BACK); glFrontFace(GL_CCW);
	glClearColor(skyboxColour.r, skyboxColour.g, skyboxColour.b, 1);
	
	return 1;
}

void updateOpenGL(){
	SDL_GetWindowSize(glWindow, &glWindowScale.x, &glWindowScale.y);
	glViewport(0, 0, glWindowScale.x, glWindowScale.y);

	HandleKeyInput();

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glUseProgram(mainShader);
	
	//projMat = projMatrix(90, (float)glWindowScale.x/glWindowScale.y, 0.1, 100); //world flipped?
	//float* worldMat = genMatrix(client.gameWorld->currPlayer->pos, client.gameWorld->currPlayer->scale, client.gameWorld->currPlayer->rot);
	
	float* viewMatTranslate = translateMatrix(defaultMatrix, vec3Mult(currentCamera.pos, (Vector3){-1, -1, 1}));
	viewMat = rotateMatrixEvil(viewMatTranslate, currentCamera.rot);
	free(viewMatTranslate);
	
	//glUniformMatrix4fv(glLocs[GLVAL_WORLDMATRIX], 1, GL_FALSE, client.gameWorld->currPlayer->transform);
	//glUniformMatrix4fv(glLocs[GLVAL_WORLDMATRIX], 1, GL_FALSE, worldMat);
	glUniformMatrix4fv(glLocs[GLVAL_PROJMATRIX], 1, GL_FALSE, currentCamera.proj);
	glUniformMatrix4fv(glLocs[GLVAL_VIEWMATRIX], 1, GL_FALSE, viewMat);
	//glDrawArrays(GL_TRIANGLES, 0, 3);
	glDrawElements(GL_TRIANGLES, playerMesh->faceCount * 3, GL_UNSIGNED_INT, 0); //segment faults on second frame
	
	free(worldMat);
	
	SDL_GL_SwapWindow(glWindow);
}

void cleanupOpenGL(){
	glDeleteProgram(mainShader);
	glDeleteBuffers(1, &VAO); glDeleteBuffers(1, &VBO); glDeleteBuffers(1, &EBO);
}

float *rotateMatrixEvil(mat4 matrix, Vector3 angle){
	float *output;
	output = malloc(sizeof(mat4));
	
	float *xMatrix = multMatrix(matrix, axisRotMatrix(1, angle.y));
	float *yMatrix = multMatrix(xMatrix, axisRotMatrix(0, angle.x));
	float *zMatrix = multMatrix(yMatrix, axisRotMatrix(2, angle.z));
	
	memcpy(output, zMatrix, sizeof(mat4));
	free(xMatrix); free(yMatrix); free(zMatrix); 
	return output;
}

void drawMeshOpenGL(Mesh* mesh, mat4 transform, SDL_FColor colour, SDL_Texture* texture){
	glUniformMatrix4fv(glLocs[GLVAL_WORLDMATRIX], 1, GL_FALSE, transform);

	float colourFloat[4] = {colour.r, colour.g, colour.b, colour.a};
	glUniform4fv(glLocs[GLVAL_MULTCOLOUR], 1, colourFloat);

	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(MeshFace) * mesh->faceCount, mesh->faces, GL_STATIC_DRAW); 
	glBufferData(GL_ARRAY_BUFFER, sizeof(MeshVert) * mesh->vertCount, mesh->verts, GL_STATIC_DRAW);
	
	glDrawElements(GL_TRIANGLES, mesh->faceCount * 3, GL_UNSIGNED_INT, 0);
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

void openGlGenBuffers(Mesh* mesh){
	float* vertArray = vertsToArray(mesh);
	mesh->vertArray = (Uint32)(*vertArray);

	//glGenVertexArrays(1, &mesh->vertArray); glBindVertexArray(mesh->vertArray);

    //glGenBuffers(1, &mesh->vertBuffer); glBindBuffer(GL_ARRAY_BUFFER, mesh->vertBuffer);
   	//glBufferData(GL_ARRAY_BUFFER, float * mesh->vertCount, vertBuffer, GL_STATIC_DRAW);

    //glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(MeshFace) * playerMesh->faceCount, playerMesh->faces, GL_STATIC_DRAW); 
	//glBufferData(GL_ARRAY_BUFFER, sizeof(MeshVert) * playerMesh->vertCount, playerMesh->verts, GL_STATIC_DRAW);

    /*glGenBuffers(1, &mesh->eleBuffer);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mesh->eleBuffer);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(MeshFace) * mesh->faceCount, mesh->faces, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 12 * sizeof(float), (void*)0); //pos
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 12 * sizeof(float), (void*)(3 * sizeof(float))); //norm
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 12 * sizeof(float), (void*)(6 * sizeof(float))); //uv
    glVertexAttribPointer(3, 4, GL_FLOAT, GL_FALSE, 12 * sizeof(float), (void*)(8 * sizeof(float))); //colour
    
    glEnableVertexAttribArray(0); 
    glEnableVertexAttribArray(1); 
    glEnableVertexAttribArray(2); 
    glEnableVertexAttribArray(3);*/
}