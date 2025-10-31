#define SDL_MAIN_USE_CALLBACKS 1
#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>
#include <SDL3_image/SDL_image.h>
//#include <iostream>

#include <stdio.h>
#include <string.h>
#include <math.h>

#include <structs.h>
#include "renderer.h"

SDL_Window *window = NULL;
SDL_Window *glWindow = NULL;
SDL_Renderer *renderer = NULL;

SDL_Point windowScaleIntent = {320, 240};
double windowScaleFactor;
SDL_Point windowScale = {640, 480};

Vector3 cameraPos;

Uint64 last = 0;
Uint64 now = 0;
double deltaTime = 0;

float timer = 0;

SDL_Texture *fontTex = NULL;

KeyMap keyList[4];

void HandleKeyInput();
SDL_Texture *newTexture(char* path);
void drawText(SDL_Renderer *renderer, SDL_Texture *texture, char* text, char charOff, short posX, short posY, short width, short height, short kern);

SDL_AppResult SDL_AppInit(void **appstate, int argc, char *argv[]){
	SDL_SetAppMetadata("Example Renderer Clear", "1.0", NULL);
	
	if (!SDL_Init(SDL_INIT_VIDEO)){
		SDL_Log("Couldn't initialize SDL: %s", SDL_GetError());
		return SDL_APP_FAILURE;
	}

	if (!SDL_CreateWindowAndRenderer("Sandblox (2D Isometric)", windowScale.x, windowScale.y, SDL_WINDOW_RESIZABLE, &window, &renderer)){
		SDL_Log("Couldn't create window/renderer: %s", SDL_GetError());
		return SDL_APP_FAILURE;
	}
	glWindow = SDL_CreateWindow("Sandblox (3D OpenGL)", windowScale.x, windowScale.y, SDL_WINDOW_RESIZABLE | SDL_WINDOW_OPENGL);
	SDL_SetWindowParent(glWindow, window); //SDL_SetWindowModal(glWindow, true);
	
	SDL_SetWindowMinimumSize(window, 320, 240);
	
	printf("eat thine shorteths\n");
	
	fontTex = newTexture("assets/font.png");
	
	SDL_SetRenderVSync(renderer, 1);

	keyList[0].scanCode = SDL_SCANCODE_UP; 
	keyList[1].scanCode = SDL_SCANCODE_DOWN; 
	keyList[2].scanCode = SDL_SCANCODE_LEFT; 
	keyList[3].scanCode = SDL_SCANCODE_RIGHT;
	
	//mikuTex = addTexture("miku.bmp");

	return SDL_APP_CONTINUE;
}	

SDL_AppResult SDL_AppEvent(void *appstate, SDL_Event *event){
	if (event->type == SDL_EVENT_QUIT){
		return SDL_APP_SUCCESS;
	}
	return SDL_APP_CONTINUE;
}

SDL_AppResult SDL_AppIterate(void *appstate){
	HandleKeyInput();
	
	last = now;
	now = SDL_GetTicks();
	deltaTime = min(((double)now - (double)last) / 1000.0f, 1);
	timer += deltaTime;
	
	SDL_GetWindowSize(window, &windowScale.x, &windowScale.y);
	windowScaleFactor = min((float)windowScale.x / windowScaleIntent.x, (float)windowScale.y / windowScaleIntent.y);
	
	SDL_SetRenderDrawColor(renderer, 20, 22, 24, SDL_ALPHA_OPAQUE);
	SDL_RenderClear(renderer);
	
	drawCube((Vector3){(2 + SDL_cos(timer)) / -2, SDL_sin(timer) + 1, (2 + SDL_cos(timer)) / -2}, (Vector3){2 + SDL_cos(timer), SDL_sin(timer) + 1, 2 + SDL_cos(timer)}, (SDL_FColor){0.6, 0.8, 1, 1});
	drawCube((Vector3){SDL_sin(timer) * 2 - 0.5, 0, SDL_cos(timer) * 2 - 0.5}, (Vector3){1, 1, 1}, (SDL_FColor){1, 0.2, 0.3, 1});

	SDL_SetRenderDrawColor(renderer, 255, 255, 255, SDL_ALPHA_OPAQUE);
	if(keyList[0].down)SDL_RenderFillRect(renderer, &(SDL_FRect){28, windowScale.y - 52, 24, 24});
	if(keyList[2].down)SDL_RenderFillRect(renderer, &(SDL_FRect){2, windowScale.y - 26, 24, 24});
	if(keyList[1].down)SDL_RenderFillRect(renderer, &(SDL_FRect){28, windowScale.y - 26, 24, 24});
	if(keyList[3].down)SDL_RenderFillRect(renderer, &(SDL_FRect){54, windowScale.y - 26, 24, 24});
	
	Uint8 guiText[256];
	sprintf(guiText, "FPS: %d", (Uint16)floor(1/deltaTime));
	drawText(renderer, fontTex, guiText, 32, 0, 0, 16, 16, 12);
	//SDL_RenderDebugText(renderer, 0, 0, guiText);
	
	SDL_RenderPresent(renderer);
	return SDL_APP_CONTINUE;
}

void SDL_AppQuit(void *appstate, SDL_AppResult result)
{
	SDL_DestroyTexture(fontTex);
}
    
void HandleKeyInput(){
	const bool* keyState = SDL_GetKeyboardState(NULL);
	for(int i = 0; i < 4; i++){
		keyList[i].down = keyState[keyList[i].scanCode];
	}
}

SDL_Texture *newTexture(char* path){
	SDL_Surface *texSurface = NULL;
	
	texSurface = IMG_Load(path);
	
	SDL_Texture *texture = SDL_CreateTextureFromSurface(renderer, texSurface);
	SDL_SetTextureScaleMode(texture, SDL_SCALEMODE_NEAREST);
	SDL_DestroySurface(texSurface);
	return texture;
}

void drawText(SDL_Renderer *renderer, SDL_Texture *texture, char* text, char charOff, short posX, short posY, short width, short height, short kern){
	for(int i=0; i<=strlen(text); i++){
		char charVal = (unsigned)text[i] - charOff;
		int xOff = (charVal % 16) * width;
		int yOff = floor((float)charVal / 16) * height;
		SDL_FRect sprRect = {xOff, yOff, width, height};
		SDL_FRect sprPos = {posX + kern * i, posY, width, height};
		SDL_RenderTexture(renderer, texture, &sprRect, &sprPos);
	}
}