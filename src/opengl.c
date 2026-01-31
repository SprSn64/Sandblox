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

extern SDL_Window *window;
extern SDL_Point windowScale;

SDL_Window *glWindow = NULL;

extern SDL_FColor skyboxColour;

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
	printf("Glew initiation succeeded!\n");
	
	//how the fuck do i get glew to work
	
	SDL_SetWindowParent(glWindow, window); //SDL_SetWindowModal(glWindow, true);
	SDL_SetWindowMinimumSize(glWindow, 320, 240);
	
	glEnable(GL_DEPTH_TEST);
	glClearColor(skyboxColour.r, skyboxColour.g, skyboxColour.b, 1);
	
	return 1;
}

void updateOpenGL(){
	glClear(GL_COLOR_BUFFER_BIT);
}