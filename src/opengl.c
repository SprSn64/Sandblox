//#define GLEW_STATIC

#include <SDL3/SDL.h>
#include <SDL3_image/SDL_image.h>
#include <GL/glew.h>
//include <GL/glut.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#include <structs.h>
#include "loader.h"
#include "renderer.h"

extern SDL_Window *window;
extern SDL_Point windowScale;

extern Mesh* playerMesh;

float triVerts[] = {
    -0.5, -0.5, 0,
     0.5, -0.5, 0,
     0,  0.5, 0
};  

const char *vertexShaderSource = NULL;
const char *fragmentShaderSource = NULL;

SDL_Window *glWindow = NULL;

extern SDL_FColor skyboxColour;

Uint32 newGlShader(const char* vertSource, const char* fragSource){
	(void)vertSource; (void)fragSource; 
	return 0;
}

bool initOpenGL(){
	glWindow = SDL_CreateWindow("Sandblox (3D OpenGL)", windowScale.x, windowScale.y, SDL_WINDOW_RESIZABLE | SDL_WINDOW_OPENGL);
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
	
	//how the fuck do i get glew to work
	
	SDL_SetWindowParent(glWindow, window); //SDL_SetWindowModal(glWindow, true);
	SDL_SetWindowMinimumSize(glWindow, 320, 240);
	
	vertexShaderSource = loadTextFile("assets/shaders/default.vert");
	unsigned int vertexShader;
	vertexShader = glCreateShader(GL_VERTEX_SHADER);
	glShaderSource(vertexShader, 1, &vertexShaderSource, NULL);
	glCompileShader(vertexShader);
	
	fragmentShaderSource = loadTextFile("assets/shaders/default.frag");
	unsigned int fragmentShader;
	fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
	glShaderSource(fragmentShader, 1, &fragmentShaderSource, NULL);
	glCompileShader(fragmentShader);
	
	unsigned int shaderProgram;
	shaderProgram = glCreateProgram();
	glAttachShader(shaderProgram, vertexShader);
	glAttachShader(shaderProgram, fragmentShader);
	glLinkProgram(shaderProgram);
	
	unsigned int VAO;
	glGenVertexArrays(1, &VAO);
	glBindVertexArray(VAO);	
	
	unsigned int VBO;
	glGenBuffers(1, &VBO);
	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	
	glBufferData(GL_ARRAY_BUFFER, sizeof(triVerts), triVerts, GL_STATIC_DRAW);
	
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);  
	
	glUseProgram(shaderProgram);
		glDeleteShader(vertexShader);
		glDeleteShader(fragmentShader);
	glBindVertexArray(VAO);
	
	SDL_GL_SetSwapInterval(1);
	//glEnable(GL_DEPTH_TEST);
	glClearColor(skyboxColour.r, skyboxColour.g, skyboxColour.b, 1);
	
	return 1;
}

void updateOpenGL(){
	glClear(GL_COLOR_BUFFER_BIT);
	glDrawArrays(GL_TRIANGLES, 0, 3);
	
	SDL_GL_SwapWindow(glWindow);
}