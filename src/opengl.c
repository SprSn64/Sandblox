//#define GLEW_STATIC

#include <SDL3/SDL.h>
#include <SDL3_image/SDL_image.h>
#include <GL/glew.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#include <structs.h>
#include "loader.h"
#include "renderer.h"
#include "math.h"

extern SDL_Window *window;
extern SDL_Point windowScale;

extern ClientData client;
extern Camera currentCamera;
extern float* defaultMatrix;

extern Mesh* playerMesh;
extern SDL_FColor skyboxColour;

Uint32 mainShader;

float triVerts[] = {
     0.5,  0.5, 0, 0, 0, 1, 0, 0, 1, 1, 1,  // top right
     0.5, -0.5, 0, 0, 0, 1, 1, 0, 1, 1, 1,  // bottom right
    -0.5, -0.5, 0, 0, 0, 1, 0, 1, 1, 1, 1,  // bottom left
    -0.5,  0.5, 0, 0, 0, 1, 1, 1, 1, 1, 1   // top left 
};
unsigned int triFaces[] = {  // note that we start from 0!
    0, 1, 3,   // first triangle
    1, 2, 3    // second triangle
};  

float* projMat;
float* viewMat;

const char *vertexShaderSource = NULL;
const char *fragmentShaderSource = NULL;

SDL_Window *glWindow = NULL;
SDL_Point glWindowScale = {640, 480};

Sint32 worldLoc, viewLoc,  projLoc;
Uint32 VAO, VBO, EBO;

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
	
	//glEnable(GL_DEPTH_TEST);
	glEnable(GL_CULL_FACE);
	glCullFace(GL_BACK);
	glFrontFace(GL_CCW);
	
	mainShader = loadShader("assets/shaders/default.vert", "assets/shaders/default.frag");
	
	glGenVertexArrays(1, &VAO); glBindVertexArray(VAO);	
	
	glGenBuffers(1, &VBO); glBindBuffer(GL_ARRAY_BUFFER, VBO);
	
	glGenBuffers(1, &EBO); glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(triFaces), triFaces, GL_STATIC_DRAW); 
	
	glBufferData(GL_ARRAY_BUFFER, sizeof(triVerts), triVerts, GL_STATIC_DRAW);
	
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 11 * sizeof(float), (void*)0); //pos
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 11 * sizeof(float), (void*)(3 * sizeof(float))); //norm
	glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 11 * sizeof(float), (void*)(6 * sizeof(float))); //uv
	glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, 11 * sizeof(float), (void*)(8 * sizeof(float))); //colour
	
	glEnableVertexAttribArray(0);  
	
	//projMat = projMatrix(90, 4/3, 0.01, 10000);
	
	viewLoc = glGetUniformLocation(mainShader, "view");
	projLoc = glGetUniformLocation(mainShader, "proj");
	
	SDL_GL_SetSwapInterval(1);
	//glEnable(GL_DEPTH_TEST);
	glClearColor(skyboxColour.r, skyboxColour.g, skyboxColour.b, 1);
	
	return 1;
}

void updateOpenGL(){
	SDL_GetWindowSize(glWindow, &glWindowScale.x, &glWindowScale.y);
	glViewport(0, 0, glWindowScale.x, glWindowScale.y);
	
	glUseProgram(mainShader);
	
	projMat = projMatrix(90, (float)glWindowScale.x/glWindowScale.y, 0.1, 100);
	
	glClear(GL_COLOR_BUFFER_BIT);
	
	float* viewMatTranslate = translateMatrix(defaultMatrix, vec3Mult(currentCamera.pos, (Vector3){-1, -1, 1}));
	viewMat = rotateMatrix(viewMatTranslate, currentCamera.rot);
	free(viewMatTranslate);
	
	glUniformMatrix4fv(projLoc, 1, GL_FALSE, projMat);
	glUniformMatrix4fv(viewLoc, 1, GL_FALSE, viewMat);
	//glDrawArrays(GL_TRIANGLES, 0, 3);
	glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
	
	free(projMat);
	
	SDL_GL_SwapWindow(glWindow);
}

void cleanupOpenGL(){
	glDeleteProgram(mainShader);
	glDeleteBuffers(1, &VAO); glDeleteBuffers(1, &VBO); glDeleteBuffers(1, &EBO);
}